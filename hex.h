// hex.h — HEX-режим (интерфейс)
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <windows.h>
#include <FL/Fl.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Menu_Item.H>

// === MappedFile ===
class MappedFile {
    HANDLE         hFile = INVALID_HANDLE_VALUE;
    HANDLE         hMap  = nullptr;
    const uint8_t* data_ = nullptr;
    size_t         size_ = 0;
public:
    MappedFile() = default;
    MappedFile(const MappedFile&) = delete;
    MappedFile& operator=(const MappedFile&) = delete;
    ~MappedFile() { close(); }

    bool open(const std::string& path);
    void close();
    const uint8_t* ptr()    const { return data_; }
    size_t         length() const { return size_; }
    bool           isOpen() const { return hFile != INVALID_HANDLE_VALUE; }
};

// === HexDoc ===
struct HexEditRecord {
    size_t               off = 0;
    std::vector<uint8_t> old_vals;
    std::vector<uint8_t> new_vals;
};

class HexDoc {
public:
    MappedFile                          file;
    std::unordered_map<size_t, uint8_t> edits;
    std::vector<HexEditRecord>          undo_stack;
    std::vector<HexEditRecord>          redo_stack;

    inline uint8_t get(size_t off) const {
        auto it = edits.find(off);
        return it != edits.end() ? it->second : file.ptr()[off];
    }
    inline bool   is_edited(size_t off) const { return edits.count(off) > 0; }
    inline bool   has_edits()           const { return !edits.empty(); }
    inline size_t length()              const { return file.length(); }
    inline bool   is_open()             const { return file.isOpen(); }
    inline bool   can_undo()            const { return !undo_stack.empty(); }
    inline bool   can_redo()            const { return !redo_stack.empty(); }

    void set_byte(size_t off, uint8_t val, bool record = true);
    void set_bytes(size_t off, const std::vector<uint8_t>& vals);
    bool undo_last();
    bool redo_last();
    void reset();
    bool save(const std::string& path);
};

// === HexMatch ===
struct HexMatch { size_t offset; size_t length; };

// === HexView ===
class HexView : public Fl_Widget {
public:
    static constexpr int BPR = 16;
    static constexpr int FONT_SIZE = 14;
    static constexpr int LEFT_PAD  = 10;
    static constexpr int OFFSET_W  = 100;
    static constexpr int HEX_W     = 16 * 3 * 8 + 8;
    static constexpr int ASCII_OFF = OFFSET_W + HEX_W;

    HexDoc*               doc_     = nullptr;
    Fl_Scrollbar*         vsb_     = nullptr;
    size_t                top_     = 0;
    size_t                cursor_  = 0;
    bool                  in_ascii_= false;
    int                   pending_ = -1;

    size_t                sel_start_  = 0;
    size_t                sel_end_    = 0;
    size_t                sel_anchor_ = 0;
    bool                  sel_active_ = false;

    std::vector<HexMatch> matches;
    int                   current_match = -1;

    HexView(int X, int Y, int W, int H, const char* L = nullptr)
        : Fl_Widget(X, Y, W, H, L) {}

    void set_doc(HexDoc* d);
    void attach_scrollbar(Fl_Scrollbar* s);
    size_t cursor_offset() const { return cursor_; }
    size_t visible_top()  const { return top_; }

    void clear_matches();
    void set_matches(const std::vector<HexMatch>& m);
    int match_count() const { return (int)matches.size(); }
    int current_match_index() const { return current_match; }

    bool has_selection() const { return sel_start_ < sel_end_; }
    void clear_selection();

    void copy_selection(bool as_hex);
    void paste_hex();
    void select_all();
    void goto_offset(size_t off, bool set_cursor = true);
    void find_next();
    void find_prev();
    void show_context_menu();

    int  handle(int e) override;
    void resize(int X, int Y, int W, int H) override;
    void draw() override;

private:
    void update_selection();
    void scroll_to(size_t off);
    void ensure_visible();
    int  handle_key();
    bool byte_at_mouse(int mx, int my, size_t* out_off, bool* out_ascii);
    static void scrollbar_cb(Fl_Widget* w, void* d);
    void sync_scrollbar();
};

// === Внешние символы (определены в tedit.cpp) ===
extern HexDoc hdoc;

extern Fl_Color COLOR_BG;
extern Fl_Color COLOR_SELECTION;
extern Fl_Color HEX_OFF_COLOR;
extern Fl_Color HEX_DATA_COLOR;
extern Fl_Color HEX_EDITED_COLOR;
extern Fl_Color HEX_ASCII_COLOR;
extern Fl_Color HEX_HEADER_COLOR;
extern Fl_Color HEX_CURSOR_BG;
extern Fl_Color HEX_MATCH_BG;
extern Fl_Color HEX_MATCH_CUR_BG;
extern Fl_Color HEX_SELECTION_BG;

bool        clipboard_set_utf8(const std::string& text);
std::string clipboard_get_utf8();
int         norm_ctrl(int k);
void        tedit_show_alert(const char* title, const char* msg);

// === Поиск в HEX ===
bool parse_hex_query(const char* s, std::vector<uint8_t>& out);
std::vector<HexMatch> do_hex_search(const std::vector<uint8_t>& needle);