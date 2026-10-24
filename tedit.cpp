// tedit.cpp — TEDIT Native v1.1
//
// Сборка (C++17 обязателен для string_view в подсветке):
//   g++ -std=c++17 -O2 tedit.cpp -o tedit.exe $(fltk-config --cxxflags) \
//     $(fltk-config --ldstaticflags) \
//     -static -static-libgcc -static-libstdc++ -lwinpthread -mwindows

#include <FL/FL.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Text_Editor.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_File_Chooser.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Scrollbar.H>
#include <FL/Fl_Widget.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Return_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Multiline_Output.H>
#include <FL/fl_ask.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Hold_Browser.H>

#include <windows.h>
#include <shellapi.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <chrono>
#include <fstream>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

// ============================================================
// Внешний вид
// ============================================================
const int MENU_H        = 25;
const int STATUS_H      = 24;
const int INIT_W        = 1100;
const int INIT_H        = 720;
const int TEXT_SIZE_DEF = 14;
const int GUTTER_W      = 56;
const size_t TEXT_MODE_LIMIT  = 64ull * 1024 * 1024;
const size_t HEX_SEARCH_LIMIT = 100000;
const size_t TEXT_HL_LIMIT    = 2000;
const size_t SYNTAX_HL_LIMIT  = 10ull * 1024 * 1024;   // 10 MB

Fl_Color COLOR_BG         = FL_BLACK;
Fl_Color COLOR_TEXT       = fl_rgb_color(220, 220, 215);
Fl_Color COLOR_GUTTER     = fl_rgb_color(20, 22, 24);
Fl_Color COLOR_GUTTER_FG  = fl_rgb_color(110, 120, 120);
Fl_Color COLOR_STATUS     = fl_rgb_color(35, 40, 45);
Fl_Color COLOR_STATUS_FG  = fl_rgb_color(200, 210, 210);
Fl_Color COLOR_STATUS_ACC = fl_rgb_color(255, 200, 120);
Fl_Color COLOR_CURSOR     = fl_rgb_color(160, 220, 160);
Fl_Color COLOR_SELECTION  = fl_rgb_color(60, 100, 80);

Fl_Color HEX_OFF_COLOR      = fl_rgb_color(120, 170, 190);
Fl_Color HEX_DATA_COLOR     = fl_rgb_color(220, 220, 215);
Fl_Color HEX_EDITED_COLOR   = fl_rgb_color(255, 160, 80);
Fl_Color HEX_ASCII_COLOR    = fl_rgb_color(160, 200, 160);
Fl_Color HEX_HEADER_COLOR   = fl_rgb_color(120, 140, 160);
Fl_Color HEX_CURSOR_BG      = fl_rgb_color(70, 100, 70);
Fl_Color HEX_MATCH_BG       = fl_rgb_color(90, 80, 30);
Fl_Color HEX_MATCH_CUR_BG   = fl_rgb_color(180, 130, 40);
Fl_Color HEX_SELECTION_BG   = fl_rgb_color(40, 80, 120);

Fl_Color DLG_BG        = fl_rgb_color(30, 34, 38);
Fl_Color DLG_TITLE_BG  = fl_rgb_color(45, 55, 60);
Fl_Color DLG_FG        = fl_rgb_color(220, 220, 215);
Fl_Color DLG_BTN       = fl_rgb_color(55, 65, 70);
Fl_Color DLG_BTN_HOVER = fl_rgb_color(75, 90, 95);
Fl_Color DLG_BTN_PRIM  = fl_rgb_color(70, 130, 90);
Fl_Color DLG_BTN_PRIM_H= fl_rgb_color(90, 160, 110);

// ============================================================
// Темы оформления
// ============================================================
enum class ThemeId : int { Dark = 0, Light, Sepia, Count };

struct ThemeColors {
    Fl_Color bg, text, gutter, gutter_fg;
    Fl_Color status, status_fg, status_acc;
    Fl_Color cursor, selection;
    Fl_Color hex_off, hex_data, hex_edited, hex_ascii, hex_header;
    Fl_Color hex_cursor_bg, hex_match_bg, hex_match_cur_bg, hex_sel_bg;
    Fl_Color syntax[11];
    Fl_Color dlg_bg, dlg_title_bg, dlg_fg, dlg_btn, dlg_btn_prim;
};

static const ThemeColors g_themes[(int)ThemeId::Count] = {
    // --- Dark ---
    {
        FL_BLACK,
        fl_rgb_color(220,220,215),
        fl_rgb_color(20,22,24),
        fl_rgb_color(110,120,120),
        fl_rgb_color(35,40,45),
        fl_rgb_color(200,210,210),
        fl_rgb_color(255,200,120),
        fl_rgb_color(160,220,160),
        fl_rgb_color(60,100,80),
        fl_rgb_color(120,170,190),
        fl_rgb_color(220,220,215),
        fl_rgb_color(255,160,80),
        fl_rgb_color(160,200,160),
        fl_rgb_color(120,140,160),
        fl_rgb_color(70,100,70),
        fl_rgb_color(90,80,30),
        fl_rgb_color(180,130,40),
        fl_rgb_color(40,80,120),
        {   // syntax A..K
            fl_rgb_color(220,220,215),
            fl_rgb_color(86,156,214),
            fl_rgb_color(78,201,176),
            fl_rgb_color(206,145,120),
            fl_rgb_color(106,153,85),
            fl_rgb_color(181,206,168),
            fl_rgb_color(197,134,192),
            fl_rgb_color(212,212,212),
            fl_rgb_color(220,220,170),
            fl_rgb_color(220,220,170),
            fl_rgb_color(220,160,100)
        },
        fl_rgb_color(30,34,38),
        fl_rgb_color(45,55,60),
        fl_rgb_color(220,220,215),
        fl_rgb_color(55,65,70),
        fl_rgb_color(70,130,90),
    },
    // --- Light ---
    {
        fl_rgb_color(250,250,248),
        fl_rgb_color(30,30,35),
        fl_rgb_color(235,235,232),
        fl_rgb_color(120,120,120),
        fl_rgb_color(225,225,222),
        fl_rgb_color(50,50,55),
        fl_rgb_color(200,100,0),
        fl_rgb_color(30,100,180),
        fl_rgb_color(180,210,240),
        fl_rgb_color(70,120,160),
        fl_rgb_color(30,30,35),
        fl_rgb_color(180,60,0),
        fl_rgb_color(40,120,60),
        fl_rgb_color(130,130,140),
        fl_rgb_color(220,235,250),
        fl_rgb_color(255,240,180),
        fl_rgb_color(255,200,80),
        fl_rgb_color(180,210,240),
        {
            fl_rgb_color(30,30,35),
            fl_rgb_color(30,80,180),
            fl_rgb_color(0,140,140),
            fl_rgb_color(180,60,40),
            fl_rgb_color(80,120,40),
            fl_rgb_color(140,80,180),
            fl_rgb_color(120,80,40),
            fl_rgb_color(80,80,80),
            fl_rgb_color(160,80,40),
            fl_rgb_color(160,80,40),
            fl_rgb_color(160,60,60)
        },
        fl_rgb_color(245,245,242),
        fl_rgb_color(220,225,230),
        fl_rgb_color(30,30,35),
        fl_rgb_color(200,205,210),
        fl_rgb_color(60,140,80),
    },
    // --- Sepia ---
    {
        fl_rgb_color(245,235,215),
        fl_rgb_color(60,45,30),
        fl_rgb_color(230,220,200),
        fl_rgb_color(130,110,80),
        fl_rgb_color(220,210,185),
        fl_rgb_color(70,55,40),
        fl_rgb_color(180,100,30),
        fl_rgb_color(80,60,40),
        fl_rgb_color(210,190,160),
        fl_rgb_color(100,80,50),
        fl_rgb_color(60,45,30),
        fl_rgb_color(180,80,20),
        fl_rgb_color(60,110,50),
        fl_rgb_color(140,120,90),
        fl_rgb_color(240,220,180),
        fl_rgb_color(220,200,140),
        fl_rgb_color(180,120,20),
        fl_rgb_color(200,180,150),
        {
            fl_rgb_color(60,45,30),
            fl_rgb_color(120,60,20),
            fl_rgb_color(50,110,100),
            fl_rgb_color(150,70,40),
            fl_rgb_color(90,110,40),
            fl_rgb_color(120,80,30),
            fl_rgb_color(140,70,30),
            fl_rgb_color(60,45,30),
            fl_rgb_color(140,60,40),
            fl_rgb_color(140,60,40),
            fl_rgb_color(120,60,30)
        },
        fl_rgb_color(240,230,210),
        fl_rgb_color(220,205,180),
        fl_rgb_color(60,45,30),
        fl_rgb_color(210,195,170),
        fl_rgb_color(60,110,70),
    },
};

static ThemeId g_theme = ThemeId::Dark;

// ============================================================
// MappedFile
// ============================================================
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

    bool open(const std::string& path) {
        close();
        hFile = CreateFileA(path.c_str(), GENERIC_READ,
                            FILE_SHARE_READ | FILE_SHARE_WRITE,
                            nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
        if (hFile == INVALID_HANDLE_VALUE) return false;
        LARGE_INTEGER sz;
        if (!GetFileSizeEx(hFile, &sz)) { close(); return false; }
        size_ = (size_t)sz.QuadPart;
        if (size_ == 0) return true;
        hMap = CreateFileMappingA(hFile, nullptr, PAGE_READONLY, 0, 0, nullptr);
        if (!hMap) { close(); return false; }
        data_ = static_cast<const uint8_t*>(MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0));
        if (!data_) { close(); return false; }
        return true;
    }
    void close() {
        if (data_) { UnmapViewOfFile(data_); data_ = nullptr; }
        if (hMap)  { CloseHandle(hMap);      hMap  = nullptr; }
        if (hFile != INVALID_HANDLE_VALUE) { CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE; }
        size_ = 0;
    }
    const uint8_t* ptr()    const { return data_; }
    size_t         length() const { return size_; }
    bool           isOpen() const { return hFile != INVALID_HANDLE_VALUE; }
};

// ============================================================
// HexDoc
// ============================================================
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

    void set_byte(size_t off, uint8_t val, bool record = true) {
        if (off >= file.length()) return;
        uint8_t orig = file.ptr()[off];
        uint8_t prev = get(off);
        if (prev == val) return;
        if (val == orig) edits.erase(off);
        else             edits[off] = val;
        if (record) {
            HexEditRecord r;
            r.off = off;
            r.old_vals.push_back(prev);
            r.new_vals.push_back(val);
            undo_stack.push_back(std::move(r));
            redo_stack.clear();
        }
    }

    // Пакетная запись [off, off+vals.size()) как ОДНА запись undo.
    void set_bytes(size_t off, const std::vector<uint8_t>& vals) {
        if (vals.empty() || off >= file.length()) return;
        size_t n = std::min(vals.size(), file.length() - off);
        if (n == 0) return;

        bool any = false;
        for (size_t i = 0; i < n; ++i)
            if (get(off + i) != vals[i]) { any = true; break; }
        if (!any) return;

        HexEditRecord r;
        r.off = off;
        r.old_vals.resize(n);
        r.new_vals.resize(n);
        for (size_t i = 0; i < n; ++i) {
            size_t pos   = off + i;
            uint8_t prev = get(pos);
            uint8_t val  = vals[i];
            uint8_t orig = file.ptr()[pos];
            if (val == orig) edits.erase(pos);
            else             edits[pos] = val;
            r.old_vals[i] = prev;
            r.new_vals[i] = val;
        }
        undo_stack.push_back(std::move(r));
        redo_stack.clear();
    }

    bool undo_last() {
        if (undo_stack.empty()) return false;
        HexEditRecord rec = std::move(undo_stack.back());
        undo_stack.pop_back();
        for (size_t i = 0; i < rec.old_vals.size(); ++i)
            set_byte(rec.off + i, rec.old_vals[i], false);
        redo_stack.push_back(std::move(rec));
        return true;
    }
    bool redo_last() {
        if (redo_stack.empty()) return false;
        HexEditRecord rec = std::move(redo_stack.back());
        redo_stack.pop_back();
        for (size_t i = 0; i < rec.new_vals.size(); ++i)
            set_byte(rec.off + i, rec.new_vals[i], false);
        undo_stack.push_back(std::move(rec));
        return true;
    }
    void reset() {
        file.close();
        edits.clear();
        undo_stack.clear();
        redo_stack.clear();
    }

    bool save(const std::string& path) {
        if (!has_edits()) return true;
        std::string tmp = path + ".tedit_tmp";
        std::ofstream out(tmp, std::ios::binary);
        if (!out) return false;

        const uint8_t* data = file.ptr();
        const size_t   len  = file.length();
        const size_t   CHUNK = 1 << 20;
        std::vector<uint8_t> buf(CHUNK);

        for (size_t pos = 0; pos < len; pos += CHUNK) {
            size_t n = std::min(CHUNK, len - pos);
            std::memcpy(buf.data(), data + pos, n);
            for (auto& kv : edits) {
                size_t off = kv.first;
                if (off >= pos && off < pos + n) buf[off - pos] = kv.second;
            }
            out.write((const char*)buf.data(), n);
        }
        out.close();
        file.close();
        if (!MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
            std::remove(tmp.c_str());
            return false;
        }
        edits.clear();
        undo_stack.clear();
        redo_stack.clear();
        return file.open(path);
    }
};

// ============================================================
// Кастомные модальные диалоги
// ============================================================
class TeditDialog {
public:
    static int show(const char* title, const char* msg, int type,
                    const char* ok_text = "OK",
                    const char* cancel_text = "Отмена",
                    std::string* io_value = nullptr,
                    const char* default_input = "") {
        int result = 1;

        int w = 480;
        int msg_lines = 1;
        for (const char* p = msg; *p; ++p) if (*p == '\n') msg_lines++;
        int h = 60 + msg_lines * 22 + 60;
        if (type == 2) h += 40;

        Fl_Window* dlg = new Fl_Window(w, h, title);
        dlg->set_modal();
        dlg->color(DLG_BG);
        dlg->begin();

        Fl_Box* title_box = new Fl_Box(0, 0, w, 32);
        title_box->box(FL_FLAT_BOX);
        title_box->color(DLG_TITLE_BG);
        title_box->labelcolor(DLG_FG);
        title_box->labelsize(14);
        title_box->labelfont(FL_HELVETICA_BOLD);
        title_box->copy_label(title);
        title_box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        Fl_Box* msg_box = new Fl_Box(20, 50, w - 40, msg_lines * 22 + 8, msg);
        msg_box->box(FL_NO_BOX);
        msg_box->labelcolor(DLG_FG);
        msg_box->labelsize(13);
        msg_box->align(FL_ALIGN_LEFT | FL_ALIGN_TOP | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);

        int y = 50 + msg_lines * 22 + 16;

        Fl_Input* input_field = nullptr;
        if (type == 2) {
            input_field = new Fl_Input(20, y, w - 40, 28);
            input_field->value(default_input);
            input_field->color(fl_rgb_color(20, 24, 28));
            input_field->textcolor(DLG_FG);
            input_field->selection_color(COLOR_SELECTION);
            input_field->textsize(13);
            y += 44;
        }

        const int BTN_W = 120, BTN_H = 32, BTN_GAP = 10;
        int num_btns = (type == 1) ? 2 : 1;
        int total_w = num_btns * BTN_W + (num_btns - 1) * BTN_GAP;
        int bx = w - 20 - total_w;
        int by = h - 50;

        Fl_Button* ok_btn = nullptr;
        Fl_Button* cancel_btn = nullptr;

        if (type == 1) {
            cancel_btn = new Fl_Button(bx, by, BTN_W, BTN_H, cancel_text);
            cancel_btn->color(DLG_BTN);
            cancel_btn->labelcolor(DLG_FG);
            cancel_btn->box(FL_FLAT_BOX);
            cancel_btn->labelsize(13);
            bx += BTN_W + BTN_GAP;
        }

        ok_btn = new Fl_Return_Button(bx, by, BTN_W, BTN_H, ok_text);
        ok_btn->color(DLG_BTN_PRIM);
        ok_btn->labelcolor(FL_WHITE);
        ok_btn->box(FL_FLAT_BOX);
        ok_btn->labelsize(13);

        dlg->end();
        dlg->hotspot(dlg);

        struct Ctx { int* result; Fl_Window* dlg; Fl_Input* input; std::string* out; };
        Ctx* ctx = new Ctx{ &result, dlg, input_field, io_value };

        ok_btn->callback([](Fl_Widget*, void* data) {
            Ctx* c = (Ctx*)data;
            *c->result = 0;
            if (c->input && c->out) *c->out = c->input->value() ? c->input->value() : "";
            c->dlg->hide();
        }, ctx);
        ok_btn->when(FL_WHEN_RELEASE);

        if (cancel_btn) {
            cancel_btn->callback([](Fl_Widget*, void* data) {
                Ctx* c = (Ctx*)data;
                *c->result = 1;
                c->dlg->hide();
            }, ctx);
        }

        dlg->callback([](Fl_Widget*, void* data) {
            Ctx* c = (Ctx*)data;
            *c->result = 1;
            c->dlg->hide();
        }, ctx);

        dlg->position(Fl::w() / 2 - w / 2, Fl::h() / 2 - h / 2);

        dlg->show();
        if (input_field) { input_field->take_focus(); input_field->insert_position(input_field->size()); }
        else             { ok_btn->take_focus(); }

        while (dlg->shown()) Fl::wait();

        delete ctx;
        Fl::delete_widget(dlg);
        return result;
    }

    static int alert(const char* t, const char* m) { return show(t, m, 0, "OK", nullptr); }
    static int info (const char* t, const char* m) { return show(t, m, 0, "OK", nullptr); }
    static int confirm(const char* t, const char* m,
                       const char* ok_text = "OK", const char* cancel_text = "Отмена") {
        return show(t, m, 1, ok_text, cancel_text);
    }
    static int input(const char* t, const char* p, std::string& out, const char* def = "") {
        return show(t, p, 2, "OK", "Отмена", &out, def);
    }
};

// ============================================================
// CalmEditor
// ============================================================
class CalmEditor : public Fl_Text_Editor {
    double phase = 0.25; int hold_frames = 0;
    static constexpr double PERIOD = 1.3, FRAME = 0.033, HOLD = 0.5, PI = 3.14159265358979323846;
    bool timer_on = false;
public:
    CalmEditor(int X, int Y, int W, int H, const char* L=nullptr)
        : Fl_Text_Editor(X, Y, W, H, L) {}
    void start_calm_cursor() {
        if (timer_on) return;
        timer_on = true;
        Fl::add_timeout(FRAME, timer_cb, this);
    }
    void stop_calm_cursor() {
        if (!timer_on) return;
        timer_on = false;
        Fl::remove_timeout(timer_cb, this);
    }
    void draw() override { Fl_Text_Editor::draw(); draw_cursor(); }
    int handle(int e) override {
        int r = Fl_Text_Editor::handle(e);
        if (e == FL_KEYBOARD || e == FL_PUSH || e == FL_DRAG) {
            hold_frames = (int)(HOLD / FRAME); phase = 0.25;
        }
        return r;
    }
private:
    void draw_cursor() {
        if (Fl::focus() != this || !buffer()) return;
        int pos = insert_position();
        int x = 0, y = 0; position_to_xy(pos, &x, &y);
        int hh = textsize() + 4;
        double a = 0.7 + 0.3 * std::sin(phase * 2.0 * PI);
        Fl_Color bg = color();
        int r1 = (bg >> 24) & 0xFF, g1 = (bg >> 16) & 0xFF, b1 = (bg >> 8) & 0xFF;
        int r = r1 + (int)((160 - r1) * a);
        int g = g1 + (int)((220 - g1) * a);
        int b = b1 + (int)((160 - b1) * a);
        fl_color(fl_rgb_color(r, g, b));
        fl_rectf(x, y, 2, hh);
    }
    static void timer_cb(void* d) {
        CalmEditor* s = static_cast<CalmEditor*>(d);
        if (s->hold_frames > 0) { s->hold_frames--; s->phase = 0.25; }
        else { s->phase += FRAME / PERIOD; if (s->phase >= 1.0) s->phase -= 1.0; }
        if (Fl::focus() == s && s->buffer()) {
            int pos = s->insert_position(), x = 0, y = 0;
            s->position_to_xy(pos, &x, &y);
            s->damage(FL_DAMAGE_EXPOSE, x, y, 3, s->textsize() + 4);
        }
        if (s->timer_on) Fl::repeat_timeout(FRAME, timer_cb, s);
    }
};

// ============================================================
// Нормализация Ctrl-комбинаций
// ============================================================
static int norm_ctrl(int k) {
    if (k >= 1 && k <= 26) return k - 1 + 'a';
    if (k >= 'A' && k <= 'Z') return k - 'A' + 'a';
    return k;
}

// ============================================================
// Clipboard (Win32 + fallback)
// ============================================================
#ifdef _WIN32
static bool win32_clipboard_set_utf8(const std::string& utf8) {
    if (utf8.empty()) return false;
    int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
    if (wlen <= 0) return false;
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (wlen + 1) * sizeof(wchar_t));
    if (!hMem) return false;
    wchar_t* p = static_cast<wchar_t*>(GlobalLock(hMem));
    if (!p) { GlobalFree(hMem); return false; }
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), p, wlen);
    p[wlen] = 0;
    GlobalUnlock(hMem);
    if (!OpenClipboard(nullptr)) { GlobalFree(hMem); return false; }
    EmptyClipboard();
    if (!SetClipboardData(CF_UNICODETEXT, hMem)) {
        CloseClipboard(); GlobalFree(hMem); return false;
    }
    CloseClipboard();
    return true;
}
#endif

static bool clipboard_set_utf8(const std::string& text) {
    if (text.empty()) return false;
#ifdef _WIN32
    if (win32_clipboard_set_utf8(text)) return true;
#endif
    Fl::copy(text.c_str(), (int)text.size(), 1);
    return true;
}

static std::string clipboard_get_utf8() {
    std::string out;
#ifdef _WIN32
    if (!OpenClipboard(nullptr)) return out;
    HANDLE h = GetClipboardData(CF_UNICODETEXT);
    if (h) {
        wchar_t* wp = static_cast<wchar_t*>(GlobalLock(h));
        if (wp) {
            int n = WideCharToMultiByte(CP_UTF8, 0, wp, -1, nullptr, 0, nullptr, nullptr);
            if (n > 1) {
                out.resize(n - 1);
                WideCharToMultiByte(CP_UTF8, 0, wp, -1, &out[0], n, nullptr, nullptr);
            }
            GlobalUnlock(h);
        }
    }
    CloseClipboard();
#endif
    return out;
}

// ============================================================
// ПОДСВЕТКА СИНТАКСИСА
// ============================================================
enum {
    ST_NORMAL = 'A', ST_KEYWORD = 'B', ST_TYPE = 'C', ST_STRING = 'D',
    ST_COMMENT = 'E', ST_NUMBER = 'F', ST_PREPROC = 'G', ST_OPER = 'H',
    ST_ESCAPE = 'I', ST_MARKER = 'J', ST_TAG = 'K'
};

enum class LangId : int {
    None = 0,
    C, CPP, CSharp, Python, JavaScript, TypeScript,
    Rust, Go, Java, Kotlin, Swift, Ruby, Lua, PHP,
    Shell, PowerShell, Batch, SQL, JSON, XML, HTML,
    CSS, YAML, TOML, INI, Markdown, Makefile, CMake,
    Count
};

struct LangData {
    std::unordered_map<std::string_view, char> words;
    bool line_slash = false;
    bool line_hash  = false;
    bool line_dash  = false;
    bool line_semi  = false;
    bool block_cmt  = false;
    bool backtick   = false;
    bool triple_q   = false;
    bool preproc    = false;
    bool xml_mode   = false;
    bool md_mode    = false;
    bool css_mode   = false;
};
static LangData g_langs[(int)LangId::Count];

static void add_words(LangData& L, char style, std::initializer_list<std::string_view> ws) {
    for (auto w : ws) L.words.emplace(w, style);
}

static void init_languages() {
    using L = LangId;
    {
        auto& d = g_langs[(int)L::C];
        d.line_slash = true; d.block_cmt = true; d.preproc = true;
        add_words(d, ST_KEYWORD, {"if","else","for","while","do","switch","case","default",
            "break","continue","return","goto","sizeof","typedef","struct","union","enum",
            "static","extern","const","volatile","register","auto","inline","restrict",
            "_Bool","_Complex","_Atomic","_Thread_local","_Alignof","_Alignas","_Static_assert"});
        add_words(d, ST_TYPE, {"void","int","char","short","long","float","double","signed","unsigned",
            "size_t","ssize_t","ptrdiff_t","wchar_t","bool","int8_t","int16_t","int32_t","int64_t",
            "uint8_t","uint16_t","uint32_t","uint64_t","FILE","NULL"});
    }
    {
        auto& d = g_langs[(int)L::CPP];
        d.line_slash = true; d.block_cmt = true; d.preproc = true;
        add_words(d, ST_KEYWORD, {"if","else","for","while","do","switch","case","default",
            "break","continue","return","goto","sizeof","typedef","struct","union","enum","class",
            "static","extern","const","constexpr","consteval","constinit","volatile","register","auto",
            "inline","virtual","override","final","explicit","friend","operator","template","typename",
            "using","namespace","public","private","protected","this","new","delete","throw","try",
            "catch","noexcept","static_assert","decltype","nullptr","true","false","co_await","co_yield","co_return",
            "static_cast","dynamic_cast","const_cast","reinterpret_cast","typeid","alignas","alignof"});
        add_words(d, ST_TYPE, {"void","int","char","short","long","float","double","signed","unsigned",
            "bool","wchar_t","char8_t","char16_t","char32_t","size_t","ptrdiff_t","std","string",
            "vector","map","unordered_map","set","unordered_set","pair","tuple","shared_ptr","unique_ptr","weak_ptr",
            "int8_t","int16_t","int32_t","int64_t","uint8_t","uint16_t","uint32_t","uint64_t"});
    }
    {
        auto& d = g_langs[(int)L::CSharp];
        d.line_slash = true; d.block_cmt = true;
        add_words(d, ST_KEYWORD, {"if","else","for","foreach","while","do","switch","case","default",
            "break","continue","return","goto","using","namespace","class","struct","interface","enum","record",
            "static","extern","const","readonly","volatile","virtual","override","abstract","sealed","partial",
            "public","private","protected","internal","this","base","new","delete","throw","try","catch","finally",
            "null","true","false","as","is","in","out","ref","params","operator","async","await","yield",
            "get","set","init","value","typeof","sizeof","nameof","lock","checked","unchecked","unsafe","fixed"});
        add_words(d, ST_TYPE, {"void","int","uint","long","ulong","short","ushort","byte","sbyte","float","double",
            "decimal","bool","char","string","object","dynamic","var","Task","List","Dictionary","IEnumerable"});
    }
    {
        auto& d = g_langs[(int)L::Python];
        d.line_hash = true; d.triple_q = true;
        add_words(d, ST_KEYWORD, {"def","class","if","elif","else","for","while","break","continue",
            "return","yield","import","from","as","try","except","finally","with","pass","raise","assert",
            "del","global","nonlocal","lambda","and","or","not","in","is","None","True","False","async","await"});
        add_words(d, ST_TYPE, {"int","float","str","bool","list","dict","set","tuple","bytes","bytearray",
            "complex","frozenset","object","type","Exception","ValueError","TypeError","KeyError",
            "print","len","range","enumerate","zip","map","filter","sum","min","max","abs","open","self"});
    }
    {
        auto& d = g_langs[(int)L::JavaScript];
        d.line_slash = true; d.block_cmt = true; d.backtick = true;
        add_words(d, ST_KEYWORD, {"var","let","const","function","class","extends","if","else","for","while",
            "do","switch","case","default","break","continue","return","new","delete","typeof","instanceof",
            "in","of","try","catch","finally","throw","async","await","yield","this","super","import","export",
            "from","as","null","undefined","true","false","void","with","debugger"});
        add_words(d, ST_TYPE, {"Object","Array","String","Number","Boolean","Symbol","BigInt","Date","RegExp",
            "Error","Map","Set","WeakMap","WeakSet","Promise","JSON","Math","console","window","document",
            "process","require","module","exports"});
    }
    {
        auto& d = g_langs[(int)L::TypeScript];
        d.line_slash = true; d.block_cmt = true; d.backtick = true;
        d.words = g_langs[(int)L::JavaScript].words;
        add_words(d, ST_KEYWORD, {"interface","type","enum","namespace","declare","abstract","implements",
            "public","private","protected","readonly","override","satisfies","keyof","infer","is","asserts"});
        add_words(d, ST_TYPE, {"any","unknown","never","string","number","boolean","symbol","bigint","object",
            "void","undefined","null","Array","Promise","Record","Partial","Required","Readonly","Pick","Omit"});
    }
    {
        auto& d = g_langs[(int)L::Rust];
        d.line_slash = true; d.block_cmt = true;
        add_words(d, ST_KEYWORD, {"fn","let","mut","const","static","if","else","match","for","while","loop",
            "break","continue","return","struct","enum","union","impl","trait","pub","use","mod","as","where",
            "move","ref","in","self","Self","super","crate","true","false","async","await","dyn","unsafe",
            "extern","type","box"});
        add_words(d, ST_TYPE, {"i8","i16","i32","i64","i128","isize","u8","u16","u32","u64","u128","usize",
            "f32","f64","bool","char","str","String","Vec","Box","Rc","Arc","Option","Result","Some","None","Ok","Err"});
    }
    {
        auto& d = g_langs[(int)L::Go];
        d.line_slash = true; d.block_cmt = true;
        add_words(d, ST_KEYWORD, {"func","var","const","type","if","else","for","range","switch","case","default",
            "break","continue","return","go","defer","chan","select","map","struct","interface","package","import",
            "fallthrough","goto","true","false","nil","iota"});
        add_words(d, ST_TYPE, {"bool","string","int","int8","int16","int32","int64","uint","uint8","uint16",
            "uint32","uint64","uintptr","byte","rune","float32","float64","complex64","complex128","error","any"});
    }
    {
        auto& d = g_langs[(int)L::Java];
        d.line_slash = true; d.block_cmt = true;
        add_words(d, ST_KEYWORD, {"if","else","for","while","do","switch","case","default","break","continue",
            "return","new","delete","throw","throws","try","catch","finally","class","interface","enum","record",
            "extends","implements","public","private","protected","static","final","abstract","synchronized",
            "volatile","transient","native","strictfp","this","super","instanceof","import","package","null",
            "true","false","var","yield","assert"});
        add_words(d, ST_TYPE, {"void","boolean","byte","short","int","long","char","float","double","String",
            "Object","Integer","Long","Double","Float","Boolean","Character","List","Map","Set","ArrayList",
            "HashMap","HashSet","Exception","RuntimeException"});
    }
    {
        auto& d = g_langs[(int)L::Kotlin];
        d.line_slash = true; d.block_cmt = true;
        add_words(d, ST_KEYWORD, {"fun","val","var","if","else","when","for","while","do","break","continue",
            "return","class","object","interface","enum","sealed","data","abstract","open","override","private",
            "public","protected","internal","companion","by","in","out","is","as","this","super","null","true",
            "false","package","import","try","catch","finally","throw","typealias","init","constructor","suspend",
            "inline","reified","noinline","crossinline","lateinit","const","tailrec"});
        add_words(d, ST_TYPE, {"Int","Long","Short","Byte","Float","Double","Boolean","Char","String","Unit",
            "Any","Nothing","Array","List","MutableList","Map","MutableMap","Set","MutableSet","Pair","Triple"});
    }
    {
        auto& d = g_langs[(int)L::Swift];
        d.line_slash = true; d.block_cmt = true;
        add_words(d, ST_KEYWORD, {"func","let","var","if","else","for","while","repeat","switch","case","default",
            "break","continue","return","class","struct","enum","protocol","extension","guard","defer","do","try",
            "catch","throw","throws","rethrows","in","is","as","self","Self","super","nil","true","false",
            "import","public","private","fileprivate","internal","open","static","final","override","mutating",
            "nonmutating","lazy","weak","unowned","async","await","actor"});
        add_words(d, ST_TYPE, {"Int","Int8","Int16","Int32","Int64","UInt","UInt8","UInt16","UInt32","UInt64",
            "Float","Double","Bool","String","Character","Void","Any","AnyObject","Array","Dictionary","Set",
            "Optional","Result","Error","Never"});
    }
    {
        auto& d = g_langs[(int)L::Ruby];
        d.line_hash = true;
        add_words(d, ST_KEYWORD, {"def","class","module","if","elsif","else","unless","while","until","for",
            "do","end","begin","rescue","ensure","retry","break","next","redo","return","yield","super","self",
            "nil","true","false","and","or","not","in","case","when","then","require","require_relative","attr_accessor",
            "attr_reader","attr_writer","private","protected","public","module_function","include","extend","prepend"});
        add_words(d, ST_TYPE, {"Integer","Float","String","Symbol","Array","Hash","Range","Proc","Lambda","Thread"});
    }
    {
        auto& d = g_langs[(int)L::Lua];
        d.line_dash = true;
        add_words(d, ST_KEYWORD, {"function","local","if","then","else","elseif","end","for","while","do","repeat",
            "until","break","return","goto","in","and","or","not","nil","true","false"});
        add_words(d, ST_TYPE, {"string","table","math","io","os","coroutine","debug","package","utf8","bit32"});
    }
    {
        auto& d = g_langs[(int)L::PHP];
        d.line_slash = true; d.line_hash = true; d.block_cmt = true;
        add_words(d, ST_KEYWORD, {"function","class","interface","trait","if","else","elseif","endif","for","foreach",
            "while","do","switch","case","default","break","continue","return","echo","print","new","delete","throw",
            "try","catch","finally","public","private","protected","static","final","abstract","const","global",
            "namespace","use","as","extends","implements","instanceof","null","true","false","this","self","parent",
            "require","require_once","include","include_once","isset","unset","empty"});
        add_words(d, ST_TYPE, {"int","float","string","bool","array","object","callable","iterable","mixed","void","null"});
    }
    {
        auto& d = g_langs[(int)L::Shell];
        d.line_hash = true;
        add_words(d, ST_KEYWORD, {"if","then","else","elif","fi","for","in","do","done","while","until","case","esac",
            "function","return","break","continue","local","export","source","alias","unset","readonly","declare",
            "true","false"});
        add_words(d, ST_TYPE, {"echo","printf","read","cd","pwd","ls","cp","mv","rm","mkdir","rmdir","cat","grep",
            "sed","awk","find","xargs","test","expr","exit","set","shift","trap","wait","kill"});
    }
    {
        auto& d = g_langs[(int)L::PowerShell];
        d.line_hash = true; d.backtick = true;
        add_words(d, ST_KEYWORD, {"function","filter","if","elseif","else","for","foreach","while","do","switch",
            "break","continue","return","try","catch","finally","throw","trap","param","begin","process","end",
            "in","not","and","or","xor","true","false","null"});
        add_words(d, ST_TYPE, {"String","Int32","Int64","Double","Boolean","Array","Hashtable","PSObject",
            "Write-Host","Write-Output","Get-Item","Get-ChildItem","Set-Item","New-Item","Remove-Item"});
    }
    {
        auto& d = g_langs[(int)L::Batch];
        d.line_semi = true;
        add_words(d, ST_KEYWORD, {"if","else","for","in","do","goto","call","exit","set","setlocal","endlocal",
            "shift","pause","echo","rem","not","exist","defined","errorlevel","equ","neq","lss","leq","gtr","geq"});
    }
    {
        auto& d = g_langs[(int)L::SQL];
        d.line_dash = true;
        add_words(d, ST_KEYWORD, {"SELECT","FROM","WHERE","INSERT","INTO","VALUES","UPDATE","SET","DELETE","CREATE",
            "DROP","ALTER","TABLE","INDEX","VIEW","JOIN","LEFT","RIGHT","INNER","OUTER","ON","AS","AND","OR","NOT",
            "NULL","IS","IN","BETWEEN","LIKE","ORDER","BY","GROUP","HAVING","LIMIT","OFFSET","UNION","ALL","DISTINCT",
            "PRIMARY","KEY","FOREIGN","REFERENCES","CONSTRAINT","DEFAULT","CASCADE","IF","EXISTS","BEGIN","COMMIT",
            "ROLLBACK","TRANSACTION","WITH","CASE","WHEN","THEN","ELSE","END","ASC","DESC"});
        add_words(d, ST_TYPE, {"INT","INTEGER","BIGINT","SMALLINT","VARCHAR","CHAR","TEXT","BOOLEAN","DATE","TIME",
            "TIMESTAMP","DECIMAL","NUMERIC","FLOAT","DOUBLE","REAL","BLOB","JSON","UUID"});
    }
    {
        auto& d = g_langs[(int)L::JSON];
        add_words(d, ST_KEYWORD, {"true","false","null"});
    }
    {
        auto& d = g_langs[(int)L::XML];
        d.xml_mode = true; d.block_cmt = true;
    }
    {
        auto& d = g_langs[(int)L::HTML];
        d.xml_mode = true; d.block_cmt = true;
    }
    {
        auto& d = g_langs[(int)L::CSS];
        d.block_cmt = true; d.css_mode = true;
    }
    {
        auto& d = g_langs[(int)L::YAML];
        d.line_hash = true;
        add_words(d, ST_KEYWORD, {"true","false","null","yes","no","on","off","~"});
    }
    {
        auto& d = g_langs[(int)L::TOML];
        d.line_hash = true;
        add_words(d, ST_KEYWORD, {"true","false"});
    }
    {
        auto& d = g_langs[(int)L::INI];
        d.line_semi = true; d.line_hash = true;
    }
    {
        auto& d = g_langs[(int)L::Markdown];
        d.md_mode = true;
    }
    {
        auto& d = g_langs[(int)L::Makefile];
        d.line_hash = true;
        add_words(d, ST_KEYWORD, {"ifeq","ifneq","ifdef","ifndef","else","endif","include","define","endef",
            "export","unexport","override","vpath"});
    }
    {
        auto& d = g_langs[(int)L::CMake];
        d.line_hash = true;
        add_words(d, ST_KEYWORD, {"if","else","elseif","endif","foreach","endforeach","while","endwhile","function",
            "endfunction","macro","endmacro","set","unset","message","include","add_executable","add_library",
            "target_link_libraries","target_include_directories","target_compile_options","find_package",
            "project","cmake_minimum_required","install","option","string","list","file","get_filename_component",
            "add_subdirectory","enable_testing","add_test","return","break","continue"});
    }
}

static LangId detect_language(const std::string& path, const char* text, int len) {
    using L = LangId;
    if (path.empty()) return L::None;
    std::string bn;
    {
        size_t s = path.find_last_of("/\\");
        bn = (s == std::string::npos) ? path : path.substr(s + 1);
    }
    std::string ext;
    {
        size_t dot = bn.find_last_of('.');
        if (dot != std::string::npos) {
            ext = bn.substr(dot + 1);
            for (auto& c : ext) c = (char)std::tolower((unsigned char)c);
        }
    }
    if (ext == "c") return L::C;
    if (ext == "h") return L::C;
    if (ext == "cpp" || ext == "cxx" || ext == "cc" || ext == "hpp" || ext == "hxx" || ext == "hh") return L::CPP;
    if (ext == "cs") return L::CSharp;
    if (ext == "py" || ext == "pyw" || ext == "pyi") return L::Python;
    if (ext == "js" || ext == "mjs" || ext == "cjs") return L::JavaScript;
    if (ext == "ts" || ext == "tsx" || ext == "jsx") return L::TypeScript;
    if (ext == "rs") return L::Rust;
    if (ext == "go") return L::Go;
    if (ext == "java") return L::Java;
    if (ext == "kt" || ext == "kts") return L::Kotlin;
    if (ext == "swift") return L::Swift;
    if (ext == "rb" || ext == "rake") return L::Ruby;
    if (ext == "lua") return L::Lua;
    if (ext == "php") return L::PHP;
    if (ext == "sh" || ext == "bash" || ext == "zsh" || ext == "ksh") return L::Shell;
    if (ext == "ps1" || ext == "psm1" || ext == "psd1") return L::PowerShell;
    if (ext == "bat" || ext == "cmd") return L::Batch;
    if (ext == "sql") return L::SQL;
    if (ext == "json") return L::JSON;
    if (ext == "xml" || ext == "svg" || ext == "xaml" || ext == "xsd") return L::XML;
    if (ext == "html" || ext == "htm") return L::HTML;
    if (ext == "css" || ext == "scss" || ext == "sass" || ext == "less") return L::CSS;
    if (ext == "yaml" || ext == "yml") return L::YAML;
    if (ext == "toml") return L::TOML;
    if (ext == "ini" || ext == "cfg" || ext == "conf" || ext == "properties") return L::INI;
    if (ext == "md" || ext == "markdown") return L::Markdown;
    if (ext == "mk" || ext == "mak") return L::Makefile;
    if (ext == "cmake") return L::CMake;

    if (bn == "Makefile" || bn == "makefile" || bn == "GNUmakefile") return L::Makefile;
    if (bn == "CMakeLists.txt" || bn == "CMakeLists") return L::CMake;
    if (bn == ".bashrc" || bn == ".zshrc" || bn == ".profile") return L::Shell;
    if (bn == "Dockerfile" || bn == "Containerfile") return L::Shell;

    if (text && len > 0) {
        while (len > 0 && (text[0] == ' ' || text[0] == '\t' || text[0] == '\n' || text[0] == '\r')) {
            ++text; --len;
        }
        if (len > 0 && text[0] == '{') return L::JSON;
        if (len > 5 && std::memcmp(text, "<?xml", 5) == 0) return L::XML;
        if (len > 0 && text[0] == '<') return L::XML;
        if (len > 2 && text[0] == '#' && text[1] == '!') return L::Shell;
    }
    return L::None;
}

static void tokenize_xml(const char* text, int len, std::vector<char>& out) {
    int i = 0;
    while (i < len) {
        char c = text[i];
        if (c == '<') {
            if (i + 3 < len && std::memcmp(text+i, "<!--", 4) == 0) {
                while (i < len && !(i + 2 < len && std::memcmp(text+i, "-->", 3) == 0)) out[i++] = ST_COMMENT;
                if (i < len) { out[i++] = ST_COMMENT; if (i < len) out[i++] = ST_COMMENT; if (i < len) out[i++] = ST_COMMENT; }
                continue;
            }
            if (i + 8 < len && std::memcmp(text+i, "<![CDATA[", 9) == 0) {
                while (i < len && !(i + 2 < len && std::memcmp(text+i, "]]>", 3) == 0)) out[i++] = ST_STRING;
                if (i < len) { out[i++] = ST_STRING; if (i < len) out[i++] = ST_STRING; if (i < len) out[i++] = ST_STRING; }
                continue;
            }
            while (i < len && text[i] != '>') {
                out[i] = ST_TAG;
                if (text[i] == '"' || text[i] == '\'') {
                    char q = text[i];
                    out[i++] = ST_TAG;
                    while (i < len && text[i] != q) {
                        out[i] = (text[i] == '\\') ? ST_ESCAPE : ST_STRING;
                        if (text[i] == '\\' && i + 1 < len) { out[i+1] = ST_ESCAPE; i += 2; }
                        else i++;
                    }
                    if (i < len) out[i++] = ST_TAG;
                    continue;
                }
                i++;
            }
            if (i < len) { out[i++] = ST_TAG; }
            continue;
        }
        i++;
    }
}

static void tokenize_css(const char* text, int len, std::vector<char>& out) {
    int i = 0;
    while (i < len) {
        char c = text[i];
        if (c == '/' && i + 1 < len && text[i+1] == '*') {
            out[i] = out[i+1] = ST_COMMENT;
            i += 2;
            while (i < len && !(text[i] == '*' && i + 1 < len && text[i+1] == '/')) out[i++] = ST_COMMENT;
            if (i < len) { out[i++] = ST_COMMENT; if (i < len) out[i++] = ST_COMMENT; }
            continue;
        }
        if (c == '"' || c == '\'') {
            char q = c;
            out[i++] = ST_STRING;
            while (i < len && text[i] != q && text[i] != '\n') {
                if (text[i] == '\\' && i + 1 < len) { out[i] = out[i+1] = ST_ESCAPE; i += 2; }
                else out[i++] = ST_STRING;
            }
            if (i < len && text[i] == q) out[i++] = ST_STRING;
            continue;
        }
        if (c == '@') {
            while (i < len && (std::isalnum((unsigned char)text[i]) || text[i] == '-' || text[i] == '@')) out[i++] = ST_KEYWORD;
            continue;
        }
        if (c == '#') {
            int j = i + 1;
            while (j < len && (std::isxdigit((unsigned char)text[j]))) j++;
            if (j > i + 1) { while (i < j) out[i++] = ST_NUMBER; continue; }
        }
        if (c == '.' || c == '#' || c == ':') {
            out[i++] = ST_TYPE;
            continue;
        }
        if (std::isdigit((unsigned char)c) || (c == '-' && i+1 < len && std::isdigit((unsigned char)text[i+1]))) {
            if (c == '-') out[i++] = ST_NUMBER;
            while (i < len && (std::isalnum((unsigned char)text[i]) || text[i] == '.' || text[i] == '%')) out[i++] = ST_NUMBER;
            continue;
        }
        if (std::isalpha((unsigned char)c) || c == '-' || c == '_') {
            int start = i;
            while (i < len && (std::isalnum((unsigned char)text[i]) || text[i] == '-' || text[i] == '_')) i++;
            int k = i;
            while (k < len && (text[k] == ' ' || text[k] == '\t')) k++;
            if (k < len && text[k] == ':') {
                for (int j = start; j < i; j++) out[j] = ST_KEYWORD;
            }
            continue;
        }
        i++;
    }
}

static void tokenize_md(const char* text, int len, std::vector<char>& out) {
    int i = 0;
    bool at_line_start = true;
    bool in_code_block = false;
    while (i < len) {
        if (at_line_start) {
            int k = i;
            while (k < len && (text[k] == ' ' || text[k] == '\t')) k++;
            if (k < len && text[k] == '#') {
                while (k < len && text[k] == '#') { out[k++] = ST_KEYWORD; }
                while (k < len && text[k] != '\n') out[k++] = ST_KEYWORD;
                i = k;
                if (i < len && text[i] == '\n') { i++; }
                at_line_start = true;
                continue;
            }
            if (k + 2 < len && text[k] == '`' && text[k+1] == '`' && text[k+2] == '`') {
                in_code_block = !in_code_block;
                while (k < len && text[k] != '\n') out[k++] = ST_STRING;
                i = k;
                if (i < len && text[i] == '\n') i++;
                at_line_start = true;
                continue;
            }
            if (k < len && text[k] == '>') {
                while (k < len && text[k] != '\n') out[k++] = ST_COMMENT;
                i = k;
                if (i < len && text[i] == '\n') i++;
                at_line_start = true;
                continue;
            }
            if (k < len && (text[k] == '-' || text[k] == '*' || text[k] == '+')) {
                out[k++] = ST_MARKER;
                while (k < len && (text[k] == ' ' || text[k] == '\t')) k++;
                i = k;
                at_line_start = false;
                continue;
            }
            if (k < len && std::isdigit((unsigned char)text[k])) {
                int kk = k;
                while (kk < len && std::isdigit((unsigned char)text[kk])) kk++;
                if (kk < len && (text[kk] == '.' || text[kk] == ')')) {
                    while (k <= kk) out[k++] = ST_MARKER;
                    i = k;
                    at_line_start = false;
                    continue;
                }
            }
            at_line_start = false;
        }
        if (in_code_block) {
            if (text[i] == '\n') { at_line_start = true; out[i++] = ST_STRING; }
            else out[i++] = ST_STRING;
            continue;
        }
        char c = text[i];
        if (c == '`') {
            out[i++] = ST_STRING;
            while (i < len && text[i] != '`' && text[i] != '\n') out[i++] = ST_STRING;
            if (i < len && text[i] == '`') out[i++] = ST_STRING;
            continue;
        }
        if (c == '*' || c == '_') {
            out[i++] = ST_MARKER;
            continue;
        }
        if (c == '[') {
            out[i++] = ST_TYPE;
            while (i < len && text[i] != ']' && text[i] != '\n') out[i++] = ST_TYPE;
            if (i < len && text[i] == ']') out[i++] = ST_TYPE;
            if (i < len && text[i] == '(') {
                out[i++] = ST_STRING;
                while (i < len && text[i] != ')' && text[i] != '\n') out[i++] = ST_STRING;
                if (i < len && text[i] == ')') out[i++] = ST_STRING;
            }
            continue;
        }
        if (c == '\n') { at_line_start = true; out[i++] = ST_NORMAL; continue; }
        i++;
    }
}

static void tokenize_buffer(const char* text, int len, LangId lang, std::vector<char>& out) {
    out.assign(len > 0 ? len : 1, ST_NORMAL);
    if (lang == LangId::None || len <= 0) return;

    if (lang == LangId::XML || lang == LangId::HTML) { tokenize_xml(text, len, out); return; }
    if (lang == LangId::CSS) { tokenize_css(text, len, out); return; }
    if (lang == LangId::Markdown) { tokenize_md(text, len, out); return; }
    if (lang == LangId::JSON) {
        int i = 0;
        while (i < len) {
            char c = text[i];
            if (c == '"') {
                out[i++] = ST_STRING;
                while (i < len && text[i] != '"') {
                    if (text[i] == '\\' && i + 1 < len) { out[i] = out[i+1] = ST_ESCAPE; i += 2; }
                    else out[i++] = ST_STRING;
                }
                if (i < len) out[i++] = ST_STRING;
                continue;
            }
            if (std::isdigit((unsigned char)c) || (c == '-' && i+1 < len && std::isdigit((unsigned char)text[i+1]))) {
                if (c == '-') out[i++] = ST_NUMBER;
                while (i < len && (std::isdigit((unsigned char)text[i]) || text[i] == '.' || text[i] == 'e' || text[i] == 'E' || text[i] == '+' || text[i] == '-'))
                    out[i++] = ST_NUMBER;
                continue;
            }
            if (std::isalpha((unsigned char)c)) {
                int start = i;
                while (i < len && std::isalnum((unsigned char)text[i])) i++;
                std::string_view w(text + start, i - start);
                if (w == "true" || w == "false" || w == "null")
                    for (int j = start; j < i; j++) out[j] = ST_KEYWORD;
                continue;
            }
            i++;
        }
        return;
    }

    LangData& L = g_langs[(int)lang];
    int i = 0;
    bool in_block = false;
    bool line_only_ws = true;

    while (i < len) {
        char c = text[i];
        char nc = (i + 1 < len) ? text[i+1] : 0;
        bool was_line_only_ws = line_only_ws;

        if (c == '\n') {
            in_block = false;
            line_only_ws = true;
            i++;
            continue;
        }
        if (c != ' ' && c != '\t') line_only_ws = false;

        if (in_block) {
            out[i] = ST_COMMENT;
            if (c == '*' && nc == '/') { out[i+1] = ST_COMMENT; in_block = false; i += 2; }
            else i++;
            continue;
        }

        if (L.line_slash && c == '/' && nc == '/') {
            while (i < len && text[i] != '\n') out[i++] = ST_COMMENT;
            continue;
        }
        if (L.line_hash && c == '#') {
            while (i < len && text[i] != '\n') out[i++] = ST_COMMENT;
            continue;
        }
        if (L.line_dash && c == '-' && nc == '-') {
            while (i < len && text[i] != '\n') out[i++] = ST_COMMENT;
            continue;
        }
        if (L.line_semi && c == ';') {
            if (was_line_only_ws) {
                while (i < len && text[i] != '\n') out[i++] = ST_COMMENT;
                continue;
            }
        }
        if (L.preproc && c == '#' && was_line_only_ws) {
            while (i < len && text[i] != '\n') out[i++] = ST_PREPROC;
            continue;
        }
        if (L.block_cmt && c == '/' && nc == '*') {
            out[i] = out[i+1] = ST_COMMENT;
            in_block = true;
            i += 2;
            continue;
        }
        if (c == '"' || c == '\'' || (L.backtick && c == '`')) {
            char q = c;
            if (L.triple_q && i + 2 < len && text[i+1] == q && text[i+2] == q) {
                out[i] = out[i+1] = out[i+2] = ST_STRING;
                i += 3;
                while (i < len) {
                    if (i + 2 < len && text[i] == q && text[i+1] == q && text[i+2] == q) {
                        out[i] = out[i+1] = out[i+2] = ST_STRING;
                        i += 3;
                        break;
                    }
                    if (text[i] == '\\' && i + 1 < len) { out[i] = out[i+1] = ST_ESCAPE; i += 2; }
                    else out[i++] = ST_STRING;
                }
                continue;
            }
            out[i++] = ST_STRING;
            while (i < len && text[i] != q && text[i] != '\n') {
                if (text[i] == '\\' && i + 1 < len) { out[i] = out[i+1] = ST_ESCAPE; i += 2; }
                else out[i++] = ST_STRING;
            }
            if (i < len && text[i] == q) out[i++] = ST_STRING;
            continue;
        }
        if (std::isdigit((unsigned char)c) || (c == '.' && std::isdigit((unsigned char)nc))) {
            while (i < len) {
                char d = text[i];
                if (std::isalnum((unsigned char)d) || d == '.' || d == '_') out[i++] = ST_NUMBER;
                else break;
            }
            continue;
        }
        if (std::isalpha((unsigned char)c) || c == '_' || c == '$' || c == '@') {
            int start = i;
            while (i < len && (std::isalnum((unsigned char)text[i]) || text[i] == '_' || text[i] == '$' || text[i] == '@'))
                i++;
            std::string_view w(text + start, i - start);
            auto it = L.words.find(w);
            char st = (it != L.words.end()) ? it->second : ST_NORMAL;
            for (int j = start; j < i; j++) out[j] = st;
            continue;
        }
        i++;
    }
}

// ============================================================
// Состояние подсветки
// ============================================================
static Fl_Text_Buffer*         g_style_buf  = nullptr;
static std::vector<char>       g_style_data;
static LangId                  g_lang        = LangId::None;
static bool                    g_syntax_dirty = false;
static Fl_Text_Display::Style_Table_Entry g_style_table[] = {
    { fl_rgb_color(220, 220, 215), FL_FREE_FONT, 14, 0 },
    { fl_rgb_color(86, 156, 214),  FL_FREE_FONT, 14, FL_BOLD },
    { fl_rgb_color(78, 201, 176),  FL_FREE_FONT, 14, 0 },
    { fl_rgb_color(206, 145, 120), FL_FREE_FONT, 14, 0 },
    { fl_rgb_color(106, 153, 85),  FL_FREE_FONT, 14, FL_ITALIC },
    { fl_rgb_color(181, 206, 168), FL_FREE_FONT, 14, 0 },
    { fl_rgb_color(197, 134, 192), FL_FREE_FONT, 14, 0 },
    { fl_rgb_color(212, 212, 212), FL_FREE_FONT, 14, 0 },
    { fl_rgb_color(220, 220, 170), FL_FREE_FONT, 14, FL_ITALIC },
    { fl_rgb_color(220, 220, 170), FL_FREE_FONT, 14, FL_BOLD },
    { fl_rgb_color(220, 160, 100), FL_FREE_FONT, 14, FL_BOLD },
};
static constexpr int G_STYLE_COUNT = sizeof(g_style_table) / sizeof(g_style_table[0]);

// ============================================================
// HexView
// ============================================================
struct HexMatch { size_t offset; size_t length; };

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

    HexView(int X, int Y, int W, int H, const char* L=nullptr)
        : Fl_Widget(X, Y, W, H, L) {}

    void set_doc(HexDoc* d) {
        doc_ = d; top_ = cursor_ = 0; pending_ = -1;
        sel_start_ = sel_end_ = sel_anchor_ = 0;
        sel_active_ = false;
        matches.clear(); current_match = -1;
        sync_scrollbar(); redraw();
    }
    void attach_scrollbar(Fl_Scrollbar* s) {
        vsb_ = s; vsb_->callback(scrollbar_cb, this); sync_scrollbar();
    }
    size_t cursor_offset() const { return cursor_; }
    size_t visible_top()  const { return top_; }

    void clear_matches() { matches.clear(); current_match = -1; redraw(); }
    void set_matches(const std::vector<HexMatch>& m) {
        matches = m; current_match = -1; redraw();
    }
    int match_count() const { return (int)matches.size(); }
    int current_match_index() const { return current_match; }

    bool has_selection() const { return sel_start_ < sel_end_; }
    void clear_selection() { sel_start_ = sel_end_ = 0; sel_active_ = false; redraw(); }

    void copy_selection(bool as_hex) {
        if (sel_start_ >= sel_end_ || !doc_) return;
        std::string s;
        s.reserve((sel_end_ - sel_start_) * (as_hex ? 3 : 1));
        if (as_hex) {
            char buf[4];
            for (size_t i = sel_start_; i < sel_end_; ++i) {
                std::snprintf(buf, sizeof(buf), "%02X ", (unsigned)doc_->get(i));
                s += buf;
            }
            if (!s.empty()) s.pop_back();
        } else {
            for (size_t i = sel_start_; i < sel_end_; ++i) {
                uint8_t b = doc_->get(i);
                s += (b >= 32 && b < 127) ? (char)b : '.';
            }
        }
        if (!s.empty()) clipboard_set_utf8(s);
    }

    void paste_hex() {
        if (!doc_ || !doc_->is_open()) return;
        std::string text = clipboard_get_utf8();
        if (text.empty()) return;
        std::vector<uint8_t> bytes;
        bytes.reserve(text.size() / 2);
        int hi = -1; bool valid = true;
        for (char c : text) {
            if (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == '-' || c == ',') continue;
            int d = -1;
            if (c >= '0' && c <= '9') d = c - '0';
            else if (c >= 'a' && c <= 'f') d = 10 + c - 'a';
            else if (c >= 'A' && c <= 'F') d = 10 + c - 'A';
            else { valid = false; break; }
            if (hi < 0) hi = d;
            else { bytes.push_back((uint8_t)((hi << 4) | d)); hi = -1; }
        }
        if (!valid || hi >= 0 || bytes.empty()) {
            TeditDialog::alert("Вставка", "В буфере нет корректной hex-последовательности.");
            return;
        }
        size_t len = doc_->length();
        if (len == 0 || cursor_ >= len) return;
        size_t maxn = std::min(bytes.size(), len - cursor_);
        if (maxn == 0) return;
        std::vector<uint8_t> chunk(bytes.begin(), bytes.begin() + (std::ptrdiff_t)maxn);
        doc_->set_bytes(cursor_, chunk);
        cursor_ = cursor_ + maxn - 1;
        ensure_visible();
        redraw();
        do_callback();
    }

    void select_all() {
        if (!doc_ || doc_->length() == 0) return;
        sel_start_ = 0;
        sel_end_ = doc_->length();
        sel_anchor_ = 0;
        cursor_ = sel_end_ - 1;
        sel_active_ = true;
        ensure_visible();
        redraw();
    }

    void goto_offset(size_t off, bool set_cursor = true) {
        if (!doc_ || !doc_->is_open()) return;
        if (off >= doc_->length()) off = doc_->length() > 0 ? doc_->length() - 1 : 0;
        if (set_cursor) cursor_ = off;
        top_ = (off / BPR) * BPR;
        sync_scrollbar();
        redraw();
    }
    void find_next() {
        if (matches.empty()) return;
        current_match = (current_match + 1) % (int)matches.size();
        goto_offset(matches[current_match].offset);
    }
    void find_prev() {
        if (matches.empty()) return;
        current_match = current_match - 1;
        if (current_match < 0) current_match = (int)matches.size() - 1;
        goto_offset(matches[current_match].offset);
    }

    void show_context_menu() {
        static Fl_Menu_Item menu[] = {
            {"Копировать HEX (Ctrl+C)",        0, 0, 0, 0},
            {"Копировать ASCII (Ctrl+Shift+C)", 0, 0, 0, 0},
            {"",                                0, 0, 0, FL_MENU_DIVIDER},
            {"Вставить HEX (Ctrl+V)",           0, 0, 0, 0},
            {"",                                0, 0, 0, FL_MENU_DIVIDER},
            {"Отменить (Ctrl+Z)",               0, 0, 0, 0},
            {"Повторить (Ctrl+Y)",              0, 0, 0, 0},
            {"",                                0, 0, 0, FL_MENU_DIVIDER},
            {"Выделить всё (Ctrl+A)",           0, 0, 0, 0},
            {0}
        };
        bool has_sel = has_selection();
        menu[0].flags = has_sel ? 0 : FL_MENU_INACTIVE;
        menu[1].flags = has_sel ? 0 : FL_MENU_INACTIVE;
        menu[5].flags = doc_->can_undo() ? 0 : FL_MENU_INACTIVE;
        menu[6].flags = doc_->can_redo() ? 0 : FL_MENU_INACTIVE;
        menu[8].flags = (doc_ && doc_->length() > 0) ? 0 : FL_MENU_INACTIVE;

        const Fl_Menu_Item* m = menu->popup(Fl::event_x_root(), Fl::event_y_root(), 0, 0, 0);
        if (!m) return;
        int idx = (int)(m - menu);
        switch (idx) {
            case 0: copy_selection(true); break;
            case 1: copy_selection(false); break;
            case 3: paste_hex(); break;
            case 5: if (doc_->undo_last()) { redraw(); do_callback(); } break;
            case 6: if (doc_->redo_last()) { redraw(); do_callback(); } break;
            case 8: select_all(); break;
        }
    }

    int handle(int e) override {
        if (!doc_ || !doc_->is_open()) return Fl_Widget::handle(e);
        switch (e) {
            case FL_MOUSEWHEEL: {
                long long d = (long long)Fl::event_dy() * 3 * BPR;
                long long nt = (long long)top_ + d;
                if (nt < 0) nt = 0;
                scroll_to((size_t)nt);
                return 1;
            }
            case FL_PUSH: {
                take_focus();
                size_t off; bool ascii;
                bool on_byte = byte_at_mouse(Fl::event_x(), Fl::event_y(), &off, &ascii);

                if (Fl::event_button() == FL_RIGHT_MOUSE) {
                    if (on_byte && !has_selection()) {
                        cursor_ = off; in_ascii_ = ascii; pending_ = -1;
                        redraw();
                    }
                    show_context_menu();
                    return 1;
                }

                if (!on_byte) return 1;

                bool shift = (Fl::event_state() & FL_SHIFT) != 0;
                if (shift) {
                    if (!sel_active_) { sel_anchor_ = cursor_; sel_active_ = true; }
                    cursor_ = off;
                    update_selection();
                    redraw();
                } else {
                    cursor_ = off; in_ascii_ = ascii; pending_ = -1;
                    sel_anchor_ = off;
                    sel_start_ = sel_end_ = 0;
                    sel_active_ = false;
                    redraw();
                }
                return 1;
            }
            case FL_DRAG: {
                size_t off; bool ascii;
                if (byte_at_mouse(Fl::event_x(), Fl::event_y(), &off, &ascii)) {
                    cursor_ = off;
                    if (!sel_active_) sel_active_ = true;
                    update_selection();
                    redraw();
                } else {
                    if (Fl::event_y() < y()) {
                        scroll_to(top_ >= BPR ? top_ - BPR : 0);
                        size_t new_off = top_ + (Fl::event_x() > x() + LEFT_PAD + OFFSET_W ? BPR - 1 : 0);
                        if (new_off < doc_->length()) {
                            cursor_ = new_off;
                            if (!sel_active_) sel_active_ = true;
                            update_selection();
                            redraw();
                        }
                    } else if (Fl::event_y() > y() + h()) {
                        fl_font(FL_COURIER, FONT_SIZE);
                        int fh = fl_height();
                        int rows = std::max(1, (h() - 2*fh) / fh);
                        scroll_to(top_ + BPR);
                        size_t new_off = top_ + (size_t)(rows - 1) * BPR;
                        if (new_off < doc_->length()) {
                            cursor_ = new_off;
                            if (!sel_active_) sel_active_ = true;
                            update_selection();
                            redraw();
                        }
                    }
                }
                return 1;
            }
            case FL_RELEASE:
                return 1;
            case FL_SHORTCUT:
            case FL_KEYBOARD:
                return handle_key();
            case FL_FOCUS:
            case FL_UNFOCUS:
                redraw();
                return 1;
        }
        return Fl_Widget::handle(e);
    }
    void resize(int X, int Y, int W, int H) override {
        Fl_Widget::resize(X, Y, W, H); sync_scrollbar();
    }

    void draw() override {
        fl_color(COLOR_BG); fl_rectf(x(), y(), w(), h());
        fl_font(FL_COURIER, FONT_SIZE);
        int fh = fl_height();
        int cw = (int)fl_width('M');
        int base_x = x() + LEFT_PAD;
        int hex_x  = base_x + OFFSET_W;
        int asc_x  = base_x + ASCII_OFF;
        int rows = std::max(1, (h() - 2*fh) / fh);

        if (!doc_ || !doc_->is_open()) {
            fl_color(fl_rgb_color(120,120,120));
            fl_draw("(файл не открыт — Ctrl+O)", x()+LEFT_PAD, y()+fh+4);
            return;
        }

        fl_color(HEX_HEADER_COLOR);
        fl_draw("offset", base_x, y()+fh);
        fl_draw("00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F", hex_x, y()+fh);
        fl_draw("ascii", asc_x, y()+fh);

        const size_t len = doc_->length();
        const size_t first_row = top_ / BPR;
        const size_t total     = (len + BPR - 1) / BPR;
        const size_t last_row  = std::min(total, first_row + rows + 1);

        uint8_t  vals[BPR];
        bool     ised[BPR];
        Fl_Color bgs[BPR];
        static const char* hx = "0123456789ABCDEF";

        for (size_t row = first_row; row < last_row; ++row) {
            const size_t base_off = row * BPR;
            const int yy = y() + (int)(row - first_row + 2) * fh;
            const int n  = (int)std::min<size_t>(BPR, len - base_off);

            int mi = -1;
            if (!matches.empty()) {
                auto it = std::upper_bound(matches.begin(), matches.end(), base_off,
                    [](size_t v, const HexMatch& m){ return v < m.offset; });
                if (it != matches.begin()) {
                    auto prev = it - 1;
                    if (prev->offset + prev->length > base_off) it = prev;
                }
                if (it != matches.end()) mi = (int)std::distance(matches.begin(), it);
            }

            for (int i = 0; i < n; ++i) {
                size_t off = base_off + i;
                auto eit = doc_->edits.find(off);
                if (eit != doc_->edits.end()) { vals[i] = eit->second; ised[i] = true; }
                else                          { vals[i] = doc_->file.ptr()[off]; ised[i] = false; }

                while (mi >= 0 && mi < (int)matches.size() &&
                       off >= matches[mi].offset + matches[mi].length) ++mi;
                int cur_mi = -1;
                if (mi >= 0 && mi < (int)matches.size() &&
                    off >= matches[mi].offset && off < matches[mi].offset + matches[mi].length) {
                    cur_mi = mi;
                }

                Fl_Color bg = 0;
                if (off == cursor_)      bg = HEX_CURSOR_BG;
                else if (cur_mi >= 0 && cur_mi == current_match) bg = HEX_MATCH_CUR_BG;
                else if (cur_mi >= 0)    bg = HEX_MATCH_BG;
                else if (sel_start_ < sel_end_ && off >= sel_start_ && off < sel_end_)
                    bg = HEX_SELECTION_BG;
                bgs[i] = bg;
            }

            {
                int i = 0;
                while (i < n) {
                    if (bgs[i] == 0) { ++i; continue; }
                    Fl_Color c = bgs[i];
                    int j = i + 1;
                    while (j < n && bgs[j] == c) ++j;
                    fl_color(c);
                    int hx1 = hex_x + i * 3 * cw - 1;
                    int hx2 = hex_x + j * 3 * cw - 1;
                    fl_rectf(hx1, yy - fh + 3, hx2 - hx1, fh);
                    int ax1 = asc_x + i * cw - 1;
                    int ax2 = asc_x + j * cw - 1;
                    fl_rectf(ax1, yy - fh + 3, ax2 - ax1 + cw, fh);
                    i = j;
                }
            }

            fl_color(HEX_OFF_COLOR);
            char ob[16];
            std::snprintf(ob, sizeof(ob), "%08llX", (unsigned long long)base_off);
            fl_draw(ob, base_x, yy);

            for (int i = 0; i < n; ++i) {
                uint8_t b = vals[i];
                Fl_Color bg = bgs[i];
                Fl_Color hcol, acol;
                if (bg == HEX_CURSOR_BG || bg == HEX_MATCH_CUR_BG) { hcol = acol = fl_rgb_color(255, 255, 220); }
                else if (bg == HEX_MATCH_BG)                       { hcol = acol = fl_rgb_color(255, 240, 160); }
                else if (ised[i])                                   { hcol = acol = HEX_EDITED_COLOR; }
                else                                                { hcol = HEX_DATA_COLOR; acol = HEX_ASCII_COLOR; }

                char pair[3] = { hx[b >> 4], hx[b & 0xF], 0 };
                fl_color(hcol);
                fl_draw(pair, hex_x + i * 3 * cw, yy);

                char asc = (b >= 32 && b < 127) ? (char)b : '.';
                char ab[2] = { asc, 0 };
                fl_color(acol);
                fl_draw(ab, asc_x + i * cw, yy);
            }
        }
    }

private:
    void update_selection() {
        if (!sel_active_) { sel_start_ = sel_end_ = 0; return; }
        size_t a = std::min(sel_anchor_, cursor_);
        size_t b = std::max(sel_anchor_, cursor_);
        sel_start_ = a;
        sel_end_   = b + 1;
    }

    void scroll_to(size_t off) { top_ = off; sync_scrollbar(); redraw(); }

    void ensure_visible() {
        fl_font(FL_COURIER, FONT_SIZE);
        int fh = fl_height();
        int rows = std::max(1, (h() - 2*fh) / fh);
        size_t vis = (size_t)rows * BPR;
        size_t cs = (cursor_ / BPR) * BPR;
        if (cs < top_) top_ = cs;
        else if (cursor_ >= top_ + vis)
            top_ = cs >= vis ? cs - vis + BPR : 0;
        sync_scrollbar();
    }

    int handle_key() {
        int k = Fl::event_key();
        int st = Fl::event_state();
        size_t len = doc_->length();
        if (len == 0) return 1;

        bool shift = (st & FL_SHIFT) != 0;
        bool ctrl  = (st & FL_CTRL) != 0;

        if (ctrl) {
            int nk = norm_ctrl(k);
            if (nk == 'z') { if (doc_->undo_last()) { redraw(); do_callback(); } return 1; }
            if (nk == 'y') { if (doc_->redo_last()) { redraw(); do_callback(); } return 1; }
            if (nk == 'c') { if (has_selection()) copy_selection(!shift); return 1; }
            if (nk == 'a') { select_all(); return 1; }
            if (nk == 'g') return 0;
            if (k == FL_Insert) { if (has_selection()) copy_selection(true); return 1; }
        }

        if (k == FL_Escape) { clear_selection(); redraw(); return 1; }

        bool is_arrow = (k == FL_Left || k == FL_Right || k == FL_Up || k == FL_Down);
        bool is_home_end = (k == FL_Home || k == FL_End);

        if (is_arrow || is_home_end) {
            if (shift) {
                if (!sel_active_) { sel_anchor_ = cursor_; sel_active_ = true; }
            } else {
                sel_active_ = false;
                sel_start_ = sel_end_ = 0;
            }
        }

        switch (k) {
            case FL_Left:  cursor_ = (cursor_ > 0) ? cursor_-1 : 0; pending_ = -1; ensure_visible(); update_selection(); redraw(); return 1;
            case FL_Right: cursor_ = std::min(len-1, cursor_+1); pending_ = -1; ensure_visible(); update_selection(); redraw(); return 1;
            case FL_Up:    cursor_ = (cursor_ >= BPR) ? cursor_-BPR : cursor_; pending_ = -1; ensure_visible(); update_selection(); redraw(); return 1;
            case FL_Down:  cursor_ = std::min(len-1, cursor_+BPR); pending_ = -1; ensure_visible(); update_selection(); redraw(); return 1;
            case FL_Page_Up: {
                fl_font(FL_COURIER, FONT_SIZE); int fh = fl_height();
                int rows = std::max(1, (h() - 2*fh) / fh);
                size_t page = (size_t)rows * BPR;
                cursor_ = (cursor_ >= page) ? cursor_-page : 0;
                top_    = (top_ >= page)    ? top_-page    : 0;
                ensure_visible(); update_selection(); redraw(); return 1;
            }
            case FL_Page_Down: {
                fl_font(FL_COURIER, FONT_SIZE); int fh = fl_height();
                int rows = std::max(1, (h() - 2*fh) / fh);
                size_t page = (size_t)rows * BPR;
                cursor_ = std::min(len-1, cursor_+page);
                top_ += page;
                ensure_visible(); update_selection(); redraw(); return 1;
            }
            case FL_Home: {
                size_t target = ctrl ? 0 : (cursor_ / BPR) * BPR;
                cursor_ = target;
                ensure_visible(); update_selection(); redraw(); return 1;
            }
            case FL_End: {
                size_t target;
                if (ctrl) target = len - 1;
                else {
                    size_t row_end = ((cursor_ / BPR) + 1) * BPR;
                    target = (row_end > len) ? len - 1 : row_end - 1;
                }
                cursor_ = target;
                ensure_visible(); update_selection(); redraw(); return 1;
            }
            case FL_Tab:
                in_ascii_ = !in_ascii_;
                redraw();
                return 1;
        }

        int ch = Fl::event_text() ? (unsigned char)Fl::event_text()[0] : 0;
        if (ch == 0) return 1;

        if (!in_ascii_) {
            int d = -1;
            if (ch >= '0' && ch <= '9') d = ch - '0';
            else if (ch >= 'a' && ch <= 'f') d = 10 + (ch - 'a');
            else if (ch >= 'A' && ch <= 'F') d = 10 + (ch - 'A');
            if (d < 0) return 1;
            if (pending_ < 0) {
                pending_ = d;
            } else {
                doc_->set_byte(cursor_, (uint8_t)((pending_ << 4) | d), true);
                pending_ = -1;
                sel_active_ = false;
                sel_start_ = sel_end_ = 0;
                cursor_ = std::min(len-1, cursor_+1);
                ensure_visible(); redraw();
                do_callback();
            }
            return 1;
        } else {
            if (ch >= 32 && ch < 127) {
                doc_->set_byte(cursor_, (uint8_t)ch, true);
                sel_active_ = false;
                sel_start_ = sel_end_ = 0;
                cursor_ = std::min(len-1, cursor_+1);
                ensure_visible(); redraw();
                do_callback();
            }
            return 1;
        }
    }

    bool byte_at_mouse(int mx, int my, size_t* out_off, bool* out_ascii) {
        fl_font(FL_COURIER, FONT_SIZE);
        int fh = fl_height();
        int cw = (int)fl_width('M');
        int rx = mx - (x() + LEFT_PAD);
        int ry = my - y() - 2*fh;
        int row = ry / fh;
        if (row < 0) return false;

        size_t base_off = (top_ / BPR + (size_t)row) * BPR;
        if (base_off >= doc_->length()) return false;

        if (rx >= OFFSET_W && rx < ASCII_OFF) {
            int rel = rx - OFFSET_W;
            int i = rel / (3 * cw);
            if (i < 0 || i >= BPR) return false;
            *out_off = base_off + i;
            *out_ascii = false;
            return (*out_off < doc_->length());
        }
        if (rx >= ASCII_OFF) {
            int i = (rx - ASCII_OFF) / cw;
            if (i < 0 || i >= BPR) return false;
            *out_off = base_off + i;
            *out_ascii = true;
            return (*out_off < doc_->length());
        }
        return false;
    }

    static void scrollbar_cb(Fl_Widget* w, void* d) {
        HexView* s = static_cast<HexView*>(d);
        s->top_ = (size_t)((Fl_Scrollbar*)w)->value();
        s->redraw();
    }
    void sync_scrollbar() {
        if (!vsb_) return;
        if (!doc_ || !doc_->is_open()) { vsb_->value(0,1,0,1); return; }
        size_t len = doc_->length();
        fl_font(FL_COURIER, FONT_SIZE);
        int fh = fl_height();
        int rows = std::max(1, (h() - 2*fh) / fh);
        size_t vis = (size_t)rows * BPR;
        double maxv = (double)std::max<size_t>(1, len);
        double page = (double)std::min<size_t>(vis, len);
        vsb_->value((double)top_, page, 0, maxv);
    }
};

// ============================================================
// Окно
// ============================================================
enum class Mode { TEXT, HEX };

class TeditWindow : public Fl_Double_Window {
public:
    Fl_Menu_Bar*  menubar     = nullptr;
    CalmEditor*   text_ed     = nullptr;
    HexView*      hex_view    = nullptr;
    Fl_Scrollbar* hex_vsb     = nullptr;
    Fl_Box*       status_pos  = nullptr;
    Fl_Box*       status_file = nullptr;
    Fl_Box*       status_size = nullptr;
    Fl_Box*       status_mode = nullptr;
    Fl_Box*       status_info = nullptr;
    Mode          mode        = Mode::TEXT;

    TeditWindow(int W, int H, const char* L) : Fl_Double_Window(W, H, L) {}

    void layout() {
        int W = w(), H = h();
        if (menubar) menubar->resize(0, 0, W, MENU_H);
        int bodyH = H - MENU_H - STATUS_H;
        if (text_ed) text_ed->resize(0, MENU_H, W, bodyH);
        const int SB_W = 16;
        if (hex_vsb)  hex_vsb->resize(W - SB_W, MENU_H, SB_W, bodyH);
        if (hex_view) hex_view->resize(0, MENU_H, W - SB_W, bodyH);

        const int POS_W = 180, MODE_W = 60, INFO_W = 300, SIZE_W = 140;
        int y = H - STATUS_H;
        if (status_pos)  status_pos->resize(0, y, POS_W, STATUS_H);
        if (status_mode) status_mode->resize(POS_W, y, MODE_W, STATUS_H);
        if (status_info) status_info->resize(POS_W+MODE_W, y, INFO_W, STATUS_H);
        if (status_file) status_file->resize(POS_W+MODE_W+INFO_W, y, W-POS_W-MODE_W-INFO_W-SIZE_W, STATUS_H);
        if (status_size) status_size->resize(W-SIZE_W, y, SIZE_W, STATUS_H);
    }
    void resize(int X, int Y, int W, int H) override {
        Fl_Double_Window::resize(X, Y, W, H); layout();
    }
    void set_mode(Mode m) {
        mode = m;
        if (m == Mode::TEXT) { hex_view->hide(); hex_vsb->hide(); text_ed->show(); }
        else                 { text_ed->hide();  hex_view->show(); hex_vsb->show(); }
        layout(); redraw();
    }
};

TeditWindow*    win = nullptr;
Fl_Text_Buffer* tbuf = nullptr;
HexDoc          hdoc;
std::string     current_path;
bool            dirty_text = false;
bool            dirty_hex  = false;
int             text_size  = TEXT_SIZE_DEF;

Fl_Preferences* prefs = nullptr;

// Forward-декларации: find_win и виджеты поиска объявлены ниже,
// в секции «Поиск», но нужны уже здесь.
extern Fl_Window* find_win;

// ============================================================
// Применение темы
// ============================================================
static void apply_theme(ThemeId id);

static void menu_theme_dark (Fl_Widget*, void*) { apply_theme(ThemeId::Dark);  }
static void menu_theme_light(Fl_Widget*, void*) { apply_theme(ThemeId::Light); }
static void menu_theme_sepia(Fl_Widget*, void*) { apply_theme(ThemeId::Sepia); }

static void apply_theme(ThemeId id) {
    if ((int)id < 0 || (int)id >= (int)ThemeId::Count) id = ThemeId::Dark;
    g_theme = id;
    const ThemeColors& c = g_themes[(int)id];

    COLOR_BG         = c.bg;
    COLOR_TEXT       = c.text;
    COLOR_GUTTER     = c.gutter;
    COLOR_GUTTER_FG  = c.gutter_fg;
    COLOR_STATUS     = c.status;
    COLOR_STATUS_FG  = c.status_fg;
    COLOR_STATUS_ACC = c.status_acc;
    COLOR_CURSOR     = c.cursor;
    COLOR_SELECTION  = c.selection;

    HEX_OFF_COLOR    = c.hex_off;
    HEX_DATA_COLOR   = c.hex_data;
    HEX_EDITED_COLOR = c.hex_edited;
    HEX_ASCII_COLOR  = c.hex_ascii;
    HEX_HEADER_COLOR = c.hex_header;
    HEX_CURSOR_BG    = c.hex_cursor_bg;
    HEX_MATCH_BG     = c.hex_match_bg;
    HEX_MATCH_CUR_BG = c.hex_match_cur_bg;
    HEX_SELECTION_BG = c.hex_sel_bg;

    DLG_BG        = c.dlg_bg;
    DLG_TITLE_BG  = c.dlg_title_bg;
    DLG_FG        = c.dlg_fg;
    DLG_BTN       = c.dlg_btn;
    DLG_BTN_PRIM  = c.dlg_btn_prim;

    for (int i = 0; i < G_STYLE_COUNT && i < 11; ++i)
        g_style_table[i].color = c.syntax[i];

    if (!win) return;

    win->color(c.bg);
    if (win->text_ed) {
        win->text_ed->color(c.bg);
        win->text_ed->textcolor(c.text);
        win->text_ed->selection_color(c.selection);
        win->text_ed->linenumber_bgcolor(c.gutter);
        win->text_ed->linenumber_fgcolor(c.gutter_fg);
        win->text_ed->redraw();
    }
    if (win->hex_view) win->hex_view->redraw();
    if (win->hex_vsb) {
        win->hex_vsb->color(c.status);
        win->hex_vsb->selection_color(c.selection);
        win->hex_vsb->redraw();
    }
    if (win->menubar) {
        win->menubar->color(c.status);
        win->menubar->textcolor(c.status_fg);
        win->menubar->redraw();
    }

    Fl_Box* cells[] = { win->status_pos, win->status_mode, win->status_info,
                        win->status_file, win->status_size };
    for (Fl_Box* b : cells) {
        if (!b) continue;
        b->color(c.status);
        b->labelcolor(c.status_fg);
    }
    if (win->status_info) win->status_info->labelcolor(c.status_acc);

    win->redraw();

    if (find_win && find_win->visible()) {
        find_win->color(c.dlg_bg);
        for (int i = 0; i < find_win->children(); ++i) {
            Fl_Widget* w = find_win->child(i);
            if (!w) continue;
            if (Fl_Input_* inp = dynamic_cast<Fl_Input_*>(w)) {
                inp->color(fl_rgb_color(20, 24, 28));
                inp->textcolor(c.dlg_fg);
                inp->selection_color(c.selection);
            } else if (Fl_Button* btn = dynamic_cast<Fl_Button*>(w)) {
                btn->color(c.dlg_btn);
                btn->labelcolor(c.dlg_fg);
            } else if (Fl_Box* box = dynamic_cast<Fl_Box*>(w)) {
                box->labelcolor(c.dlg_fg);
            }
        }
        find_win->redraw();
    }

    if (win->menubar) {
        const char* paths[] = {
            "&Вид/&Тема/&Тёмная",
            "&Вид/&Тема/&Светлая",
            "&Вид/&Тема/&Сепия",
        };
        for (int i = 0; i < 3; ++i) {
            Fl_Menu_Item* mi = const_cast<Fl_Menu_Item*>(win->menubar->find_item(paths[i]));
            if (!mi) continue;
            if (i == (int)g_theme) mi->set();
            else                   mi->clear();
        }
    }
}

// --- Поиск ---
struct TextSearchState {
    std::string term;
    bool match_case = false;
    bool dirty = true;
    std::vector<std::pair<int,int>> matches;
    int current = -1;
};
TextSearchState text_search;

std::string hex_search_term;
bool        hex_search_as_hex = true;

Fl_Window*       find_win        = nullptr;
Fl_Input*        find_input      = nullptr;
Fl_Input*        replace_input   = nullptr;
Fl_Check_Button* find_match_case = nullptr;
Fl_Check_Button* find_as_hex     = nullptr;
Fl_Box*          find_info       = nullptr;
Fl_Box*          replace_lbl     = nullptr;
Fl_Button*       b_replace       = nullptr;
Fl_Button*       b_replace_all   = nullptr;

// ============================================================
// Утилиты
// ============================================================
std::string basename_of(const std::string& p) {
    size_t s = p.find_last_of("/\\");
    return s == std::string::npos ? p : p.substr(s+1);
}
bool is_dirty() { return win->mode == Mode::TEXT ? dirty_text : dirty_hex; }

void update_title() {
    std::string name = current_path.empty() ? "untitled" : basename_of(current_path);
    std::string title = "TEDIT — " + name;
    if (is_dirty()) title += "  •";
    win->copy_label(title.c_str());
}

// ============================================================
// Кодировки и переводы строк (текстовый режим)
// ============================================================
enum class TextEnc : int { ASCII = 0, UTF8, UTF8_BOM, UTF16_LE, UTF16_BE, CP1251, Latin1 };
enum class EolKind : int { LF = 0, CRLF, CR };

struct TextFileInfo {
    TextEnc enc = TextEnc::UTF8;
    EolKind eol = EolKind::LF;
};
static TextFileInfo g_text_info;

static const char* enc_name(TextEnc e) {
    switch (e) {
        case TextEnc::ASCII:    return "ASCII";
        case TextEnc::UTF8:     return "UTF-8";
        case TextEnc::UTF8_BOM: return "UTF-8 BOM";
        case TextEnc::UTF16_LE: return "UTF-16 LE";
        case TextEnc::UTF16_BE: return "UTF-16 BE";
        case TextEnc::CP1251:   return "CP1251";
        case TextEnc::Latin1:   return "Latin-1";
    }
    return "?";
}

static const char* eol_name(EolKind e) {
    return e == EolKind::LF ? "LF" : (e == EolKind::CRLF ? "CRLF" : "CR");
}

static int enc_bom_len(TextEnc e) {
    if (e == TextEnc::UTF8_BOM) return 3;
    if (e == TextEnc::UTF16_LE || e == TextEnc::UTF16_BE) return 2;
    return 0;
}

static bool is_ascii_only(const uint8_t* p, size_t n) {
    for (size_t i = 0; i < n; ++i) if (p[i] & 0x80) return false;
    return true;
}

static bool is_valid_utf8(const uint8_t* p, size_t n) {
    size_t i = 0;
    while (i < n) {
        uint8_t c = p[i];
        if (c < 0x80) { ++i; continue; }
        int extra; uint32_t cp;
        if ((c & 0xE0) == 0xC0)      { extra = 1; cp = c & 0x1F; }
        else if ((c & 0xF0) == 0xE0) { extra = 2; cp = c & 0x0F; }
        else if ((c & 0xF8) == 0xF0) { extra = 3; cp = c & 0x07; }
        else return false;
        if (i + (size_t)extra >= n) return false;
        for (int k = 1; k <= extra; ++k) {
            uint8_t cc = p[i + k];
            if ((cc & 0xC0) != 0x80) return false;
            cp = (cp << 6) | (cc & 0x3F);
        }
        if (extra == 1 && cp < 0x80)    return false;
        if (extra == 2 && cp < 0x800)   return false;
        if (extra == 3 && cp < 0x10000) return false;
        if (cp > 0x10FFFF)              return false;
        if (cp >= 0xD800 && cp <= 0xDFFF) return false;
        i += (size_t)extra + 1;
    }
    return true;
}

static TextFileInfo detect_text_info(const uint8_t* p, size_t n) {
    TextFileInfo r;
    if (n >= 3 && p[0] == 0xEF && p[1] == 0xBB && p[2] == 0xBF) {
        r.enc = TextEnc::UTF8_BOM; p += 3; n -= 3;
    } else if (n >= 2 && p[0] == 0xFF && p[1] == 0xFE) {
        r.enc = TextEnc::UTF16_LE; p += 2; n -= 2;
    } else if (n >= 2 && p[0] == 0xFE && p[1] == 0xFF) {
        r.enc = TextEnc::UTF16_BE; p += 2; n -= 2;
    } else if (is_ascii_only(p, n)) {
        r.enc = TextEnc::ASCII;
    } else if (is_valid_utf8(p, n)) {
        r.enc = TextEnc::UTF8;
    } else {
        r.enc = TextEnc::CP1251;
    }

    size_t crlf = 0, lf = 0, cr = 0;
    for (size_t i = 0; i < n; ++i) {
        uint8_t c = p[i];
        if (c == '\r') {
            if (i + 1 < n && p[i+1] == '\n') { ++crlf; ++i; }
            else ++cr;
        } else if (c == '\n') ++lf;
    }
    if (crlf > 0 && crlf >= lf && crlf >= cr) r.eol = EolKind::CRLF;
    else if (cr > lf && cr > crlf)            r.eol = EolKind::CR;
    else                                      r.eol = EolKind::LF;
    return r;
}

static bool decode_to_utf8(const uint8_t* p, size_t n, TextEnc e, std::string& out) {
    out.clear();
    switch (e) {
        case TextEnc::ASCII:
        case TextEnc::UTF8:
        case TextEnc::UTF8_BOM:
            out.assign((const char*)p, n);
            return true;

        case TextEnc::CP1251:
        case TextEnc::Latin1: {
            if (n == 0) return true;
            UINT cp = (e == TextEnc::CP1251) ? 1251u : 28591u;
            int wlen = MultiByteToWideChar(cp, 0, (const char*)p, (int)n, nullptr, 0);
            if (wlen <= 0) return false;
            std::wstring w((size_t)wlen, L'\0');
            MultiByteToWideChar(cp, 0, (const char*)p, (int)n, &w[0], wlen);
            int u8 = WideCharToMultiByte(CP_UTF8, 0, w.data(), wlen, nullptr, 0, nullptr, nullptr);
            if (u8 <= 0) return false;
            out.resize((size_t)u8);
            WideCharToMultiByte(CP_UTF8, 0, w.data(), wlen, &out[0], u8, nullptr, nullptr);
            return true;
        }

        case TextEnc::UTF16_LE:
        case TextEnc::UTF16_BE: {
            if (n % 2 != 0) return false;
            size_t wc = n / 2;
            std::wstring w(wc, L'\0');
            for (size_t i = 0; i < wc; ++i) {
                uint16_t a = p[i*2], b = p[i*2+1];
                w[i] = (wchar_t)((e == TextEnc::UTF16_LE)
                                 ? (uint16_t)(a | (b << 8))
                                 : (uint16_t)(b | (a << 8)));
            }
            int u8 = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)wc, nullptr, 0, nullptr, nullptr);
            if (u8 <= 0) return false;
            out.resize((size_t)u8);
            WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)wc, &out[0], u8, nullptr, nullptr);
            return true;
        }
    }
    return false;
}

static bool encode_from_utf8(const std::string& utf8, TextEnc e, std::string& out) {
    out.clear();
    switch (e) {
        case TextEnc::ASCII: {
            out.reserve(utf8.size());
            for (unsigned char c : utf8) out.push_back(c < 0x80 ? (char)c : '?');
            return true;
        }
        case TextEnc::UTF8:
            out = utf8; return true;
        case TextEnc::UTF8_BOM:
            out = "\xEF\xBB\xBF" + utf8; return true;

        case TextEnc::CP1251:
        case TextEnc::Latin1: {
            if (utf8.empty()) return true;
            int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
            if (wlen <= 0) return false;
            std::wstring w((size_t)wlen, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &w[0], wlen);
            UINT cp = (e == TextEnc::CP1251) ? 1251u : 28591u;
            char defch = '?'; BOOL used = FALSE;
            int n = WideCharToMultiByte(cp, 0, w.data(), wlen, nullptr, 0, &defch, &used);
            if (n <= 0) return false;
            out.resize((size_t)n);
            WideCharToMultiByte(cp, 0, w.data(), wlen, &out[0], n, &defch, &used);
            return true;
        }

        case TextEnc::UTF16_LE:
        case TextEnc::UTF16_BE: {
            if (utf8.empty()) {
                out.push_back((char)(e == TextEnc::UTF16_LE ? 0xFF : 0xFE));
                out.push_back((char)(e == TextEnc::UTF16_LE ? 0xFE : 0xFF));
                return true;
            }
            int wlen = MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), nullptr, 0);
            if (wlen <= 0) return false;
            std::wstring w((size_t)wlen, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), (int)utf8.size(), &w[0], wlen);

            out.reserve((size_t)wlen * 2 + 2);
            if (e == TextEnc::UTF16_LE) { out.push_back((char)0xFF); out.push_back((char)0xFE); }
            else                        { out.push_back((char)0xFE); out.push_back((char)0xFF); }
            for (wchar_t c : w) {
                uint16_t u = (uint16_t)c;
                char lo = (char)(u & 0xFF), hi = (char)((u >> 8) & 0xFF);
                if (e == TextEnc::UTF16_LE) { out.push_back(lo); out.push_back(hi); }
                else                        { out.push_back(hi); out.push_back(lo); }
            }
            return true;
        }
    }
    return false;
}

static std::string normalize_eol_to_lf(const std::string& s) {
    std::string r; r.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        char c = s[i];
        if (c == '\r') {
            if (i + 1 < s.size() && s[i+1] == '\n') ++i;
            r.push_back('\n');
        } else r.push_back(c);
    }
    return r;
}

static std::string apply_eol(const std::string& s, EolKind e) {
    if (e == EolKind::LF) return s;
    const char* rep = (e == EolKind::CRLF) ? "\r\n" : "\r";
    size_t rl = (e == EolKind::CRLF) ? 2 : 1;
    std::string r; r.reserve(s.size() + s.size() / 16);
    for (char c : s) {
        if (c == '\n') r.append(rep, rl);
        else r.push_back(c);
    }
    return r;
}

static bool write_file_atomic(const std::string& path, const std::string& bytes) {
    std::string tmp = path + ".tedit_tmp";
    std::ofstream out(tmp, std::ios::binary | std::ios::trunc);
    if (!out) return false;
    out.write(bytes.data(), (std::streamsize)bytes.size());
    out.close();
    if (!out) { std::remove(tmp.c_str()); return false; }
    if (!MoveFileExA(tmp.c_str(), path.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        std::remove(tmp.c_str());
        return false;
    }
    return true;
}

void update_find_info();

static std::string g_flash_msg;
static double      g_flash_until = 0;

static double now_seconds() {
    using namespace std::chrono;
    return duration_cast<duration<double>>(steady_clock::now().time_since_epoch()).count();
}

static void set_flash(const std::string& msg, double secs = 2.5) {
    g_flash_msg = msg;
    g_flash_until = now_seconds() + secs;
}

void update_status() {
    uint64_t bytes = 0;
    if (win->mode == Mode::TEXT) bytes = (uint64_t)tbuf->length();
    else if (hdoc.is_open())     bytes = hdoc.length();
    char sb[64];
    if (bytes < 1024) std::snprintf(sb, sizeof(sb), "%llu B", (unsigned long long)bytes);
    else if (bytes < 1024ull*1024) std::snprintf(sb, sizeof(sb), "%.1f KB", bytes/1024.0);
    else if (bytes < 1024ull*1024*1024) std::snprintf(sb, sizeof(sb), "%.2f MB", bytes/1024.0/1024.0);
    else std::snprintf(sb, sizeof(sb), "%.2f GB", bytes/1024.0/1024.0/1024.0);
    win->status_size->copy_label(sb);

    win->status_file->copy_label(current_path.empty() ? "untitled" : basename_of(current_path).c_str());
    win->status_mode->copy_label(win->mode == Mode::TEXT ? "TEXT" : "HEX");

    char pb[96];
    if (win->mode == Mode::TEXT) {
        int pos = win->text_ed->insert_position();
        int line = tbuf->count_lines(0, pos) + 1;
        int col  = pos - tbuf->line_start(pos) + 1;
        std::snprintf(pb, sizeof(pb), "Ln %d, Col %d", line, col);
    } else if (hdoc.is_open()) {
        size_t off = win->hex_view->cursor_offset();
        std::snprintf(pb, sizeof(pb), "0x%08llX", (unsigned long long)off);
    } else {
        pb[0] = 0;
    }
    win->status_pos->copy_label(pb);

    if (win->mode == Mode::HEX && hdoc.is_open()) {
        char ib[200];
        int total = win->hex_view->match_count();
        int cur   = win->hex_view->current_match_index();
        int sel   = (int)(win->hex_view->sel_end_ - win->hex_view->sel_start_);
        if (total > 0 && sel > 0)
            std::snprintf(ib, sizeof(ib), "совпад: %d / %d   sel: %d   правок: %d", cur+1, total, sel, (int)hdoc.edits.size());
        else if (total > 0)
            std::snprintf(ib, sizeof(ib), "совпад: %d / %d   правок: %d", cur+1, total, (int)hdoc.edits.size());
        else if (sel > 0)
            std::snprintf(ib, sizeof(ib), "sel: %d   правок: %d", sel, (int)hdoc.edits.size());
        else
            std::snprintf(ib, sizeof(ib), "правок: %d", (int)hdoc.edits.size());
        win->status_info->copy_label(ib);
    } else if (win->mode == Mode::TEXT) {
        char ib[220];
        if (!text_search.matches.empty()) {
            int cur = text_search.current >= 0 ? text_search.current + 1 : 0;
            std::snprintf(ib, sizeof(ib), "%s · %s   совпад: %d / %d",
                          enc_name(g_text_info.enc), eol_name(g_text_info.eol),
                          cur, (int)text_search.matches.size());
        } else {
            std::snprintf(ib, sizeof(ib), "%s · %s",
                          enc_name(g_text_info.enc), eol_name(g_text_info.eol));
        }
        win->status_info->copy_label(ib);
    } else {
        win->status_info->copy_label("");
    }
    if (!g_flash_msg.empty()) {
        if (now_seconds() < g_flash_until) {
            win->status_file->copy_label(g_flash_msg.c_str());
        } else {
            g_flash_msg.clear();
        }
    }
}

static int  g_last_status_pos  = -1;
static Mode g_last_status_mode = Mode::TEXT;

void status_tick(void*) {
    if (win && win->visible()) {
        int cur = -1;
        if (win->mode == Mode::TEXT && win->text_ed)
            cur = win->text_ed->insert_position();
        else if (win->mode == Mode::HEX && win->hex_view)
            cur = (int)win->hex_view->cursor_offset();

        bool flash_active = !g_flash_msg.empty();
        if (cur != g_last_status_pos || win->mode != g_last_status_mode || flash_active) {
            update_status();
            g_last_status_pos  = cur;
            g_last_status_mode = win->mode;
        }
    }
    Fl::repeat_timeout(0.25, status_tick);
}

// ============================================================
// Подсветка синтаксиса — применение
// ============================================================
static void apply_syntax_highlight() {
    if (!tbuf || !g_style_buf) return;
    int len = tbuf->length();
    if (len > (int)SYNTAX_HL_LIMIT) return;

    if (len == 0) {
        g_style_buf->text("");
        g_syntax_dirty = false;
        if (win && win->text_ed) win->text_ed->redraw();
        return;
    }

    const char* text = tbuf->address(0);
    if (!text) return;

    tokenize_buffer(text, len, g_lang, g_style_data);
    g_style_data.push_back('\0');
    g_style_buf->text(g_style_data.data());

    if (win && win->text_ed) win->text_ed->redraw();
    g_syntax_dirty = false;
}

static void syntax_debounce_cb(void*) {
    if (g_syntax_dirty) apply_syntax_highlight();
}

static void schedule_syntax_highlight() {
    g_syntax_dirty = true;
    Fl::remove_timeout(syntax_debounce_cb);
    Fl::add_timeout(0.12, syntax_debounce_cb);
}

// ============================================================
// Текстовый поиск
// ============================================================
static std::vector<std::pair<int,int>> text_find_all(const std::string& term, bool match_case) {
    std::vector<std::pair<int,int>> res;
    if (term.empty()) return res;
    int tlen = tbuf->length();
    int nlen = (int)term.size();
    if (nlen > tlen) return res;
    const char* text = tbuf->address(0);
    if (!text) return res;

    if (match_case) {
        const char* p = text;
        const char* end = text + tlen - nlen + 1;
        while (p < end) {
            p = (const char*)std::memchr(p, term[0], end - p);
            if (!p) break;
            if (std::memcmp(p, term.c_str(), nlen) == 0) {
                res.push_back({ (int)(p - text), nlen });
                if (res.size() >= HEX_SEARCH_LIMIT) break;
                p += nlen;
            } else ++p;
        }
    } else {
        char c0 = (char)std::tolower((unsigned char)term[0]);
        int i = 0;
        while (i + nlen <= tlen) {
            const char* p = (const char*)std::memchr(text + i, term[0], tlen - i - nlen + 1);
            const char* p2 = (term[0] != c0) ? (const char*)std::memchr(text + i, c0, tlen - i - nlen + 1) : p;
            if (!p && !p2) break;
            if (!p) p = p2;
            else if (p2 && p2 < p) p = p2;
            int pos = (int)(p - text);
            bool match = true;
            for (int k = 0; k < nlen; ++k) {
                if (std::tolower((unsigned char)text[pos + k]) != std::tolower((unsigned char)term[k])) {
                    match = false; break;
                }
            }
            if (match) {
                res.push_back({ pos, nlen });
                if (res.size() >= HEX_SEARCH_LIMIT) break;
                i = pos + nlen;
            } else {
                i = pos + 1;
            }
        }
    }
    return res;
}

void refresh_text_highlights() {
    if (!tbuf) return;
    tbuf->unhighlight();
    size_t n = std::min(text_search.matches.size(), TEXT_HL_LIMIT);
    for (size_t i = 0; i < n; ++i) {
        int s = text_search.matches[i].first;
        int e = text_search.matches[i].first + text_search.matches[i].second;
        if (s < e) tbuf->highlight(s, e);
    }
}

void text_find_next(bool forward) {
    if (!find_input) return;
    std::string term = find_input->value() ? find_input->value() : "";
    if (term.empty()) { fl_beep(); return; }
    bool match_case = find_match_case && find_match_case->value();

    if (text_search.dirty || text_search.term != term || text_search.match_case != match_case) {
        text_search.term = term;
        text_search.match_case = match_case;
        text_search.matches = text_find_all(term, match_case);
        text_search.current = -1;
        text_search.dirty = false;
        refresh_text_highlights();
    }

    if (text_search.matches.empty()) { fl_beep(); update_find_info(); update_status(); return; }

    int cur_pos = win->text_ed->insert_position();
    bool first = (text_search.current < 0);

    if (forward) {
        int next = -1;
        for (size_t i = 0; i < text_search.matches.size(); ++i) {
            int off = text_search.matches[i].first;
            if (first ? (off >= cur_pos) : (off > cur_pos)) { next = (int)i; break; }
        }
        if (next < 0) next = 0;
        text_search.current = next;
    } else {
        int prev = -1;
        for (int i = (int)text_search.matches.size() - 1; i >= 0; --i) {
            int off = text_search.matches[i].first;
            if (first ? (off <= cur_pos) : (off < cur_pos)) { prev = i; break; }
        }
        if (prev < 0) prev = (int)text_search.matches.size() - 1;
        text_search.current = prev;
    }

    auto& m = text_search.matches[text_search.current];
    tbuf->select(m.first, m.first + m.second);
    win->text_ed->insert_position(m.first);
    win->text_ed->show_insert_position();
    win->text_ed->take_focus();
    update_find_info();
    update_status();
}

void text_replace_current() {
    if (!find_input || !replace_input) return;
    std::string term = find_input->value() ? find_input->value() : "";
    std::string repl = replace_input->value() ? replace_input->value() : "";
    if (term.empty()) { fl_beep(); return; }

    int s = 0, e = 0;
    if (tbuf->selection_position(&s, &e) && (e - s) == (int)term.size()) {
        char* sel = tbuf->text_range(s, e);
        bool match = false;
        if (sel) {
            if (strcmp(sel, term.c_str()) == 0) match = true;
            else if (!(find_match_case && find_match_case->value())) {
                match = true;
                for (int i = 0; i < (int)term.size(); ++i)
                    if (std::tolower((unsigned char)sel[i]) != std::tolower((unsigned char)term[i])) { match = false; break; }
            }
            free(sel);
        }
        if (match) {
            tbuf->replace(s, e, repl.c_str());
            text_search.dirty = true;
            text_search.current = -1;
            win->text_ed->insert_position(s + (int)repl.size());
            text_find_next(true);
            return;
        }
    }
    text_find_next(true);
}

void text_replace_all() {
    if (!find_input || !replace_input) return;
    std::string term = find_input->value() ? find_input->value() : "";
    std::string repl = replace_input->value() ? replace_input->value() : "";
    if (term.empty()) { fl_beep(); return; }
    bool match_case = find_match_case && find_match_case->value();

    auto matches = text_find_all(term, match_case);
    if (matches.empty()) { fl_beep(); return; }

    for (int i = (int)matches.size() - 1; i >= 0; --i)
        tbuf->replace(matches[i].first, matches[i].first + matches[i].second, repl.c_str());

    text_search.matches.clear();
    text_search.current = -1;
    text_search.dirty = true;
    tbuf->unhighlight();
    update_find_info();
    update_status();

    char buf[64];
    std::snprintf(buf, sizeof(buf), "Заменено %d вхождений.", (int)matches.size());
    TeditDialog::info("Замена", buf);
}

// ============================================================
// Поиск в HEX
// ============================================================
bool parse_hex_query(const char* s, std::vector<uint8_t>& out) {
    out.clear();
    int hi = -1;
    for (const char* p = s; *p; ++p) {
        if (*p == ' ' || *p == '\t' || *p == '-' || *p == ',') continue;
        int d = -1;
        if (*p >= '0' && *p <= '9') d = *p - '0';
        else if (*p >= 'a' && *p <= 'f') d = 10 + *p - 'a';
        else if (*p >= 'A' && *p <= 'F') d = 10 + *p - 'A';
        else return false;
        if (hi < 0) hi = d;
        else { out.push_back((uint8_t)((hi << 4) | d)); hi = -1; }
    }
    return hi < 0 && !out.empty();
}

std::vector<HexMatch> do_hex_search(const std::vector<uint8_t>& needle) {
    std::vector<HexMatch> result;
    if (needle.empty() || !hdoc.is_open()) return result;
    const size_t len  = hdoc.length();
    const size_t nlen = needle.size();
    if (nlen > len) return result;

    const uint8_t* data = hdoc.file.ptr();
    const bool has_edits = hdoc.has_edits();
    const uint8_t first = needle[0];

    auto matches_at = [&](size_t pos) -> bool {
        if (pos + nlen > len) return false;
        if (hdoc.get(pos) != first) return false;
        if (!has_edits) {
            for (size_t i = 1; i < nlen; ++i)
                if (data[pos + i] != needle[i]) return false;
        } else {
            for (size_t i = 1; i < nlen; ++i)
                if (hdoc.get(pos + i) != needle[i]) return false;
        }
        return true;
    };

    size_t off = 0;
    while (off + nlen <= len && result.size() < HEX_SEARCH_LIMIT) {
        const uint8_t* p = (const uint8_t*)std::memchr(data + off, first, len - off - nlen + 1);
        if (!p) break;
        size_t pos = (size_t)(p - data);
        if (has_edits ? (hdoc.get(pos) == first) : (data[pos] == first)) {
            if (matches_at(pos)) { result.push_back({ pos, nlen }); off = pos + nlen; continue; }
        }
        off = pos + 1;
    }

    if (has_edits) {
        for (const auto& kv : hdoc.edits) {
            size_t pos = kv.first;
            if (kv.second != first) continue;
            if (pos + nlen > len) continue;
            if (data[pos] == first) continue;
            if (matches_at(pos)) {
                bool dup = false;
                for (const auto& m : result) if (m.offset == pos) { dup = true; break; }
                if (!dup) result.push_back({ pos, nlen });
                if (result.size() >= HEX_SEARCH_LIMIT) break;
            }
        }
        std::sort(result.begin(), result.end(),
                  [](const HexMatch& a, const HexMatch& b){ return a.offset < b.offset; });
    }
    return result;
}

void run_hex_search() {
    if (!hdoc.is_open()) { TeditDialog::alert("Поиск", "Сначала откройте файл."); return; }
    const char* v = find_input && find_input->value() ? find_input->value() : "";
    if (!v[0]) {
        win->hex_view->clear_matches();
        update_find_info();
        update_status();
        return;
    }
    std::vector<uint8_t> needle;
    if (find_as_hex && find_as_hex->value()) {
        if (!parse_hex_query(v, needle)) {
            TeditDialog::alert("Некорректный запрос",
                "Ожидается последовательность hex-байтов.\nПример: FF 00 AB");
            return;
        }
    } else {
        needle.assign((const uint8_t*)v, (const uint8_t*)v + std::strlen(v));
    }
    auto res = do_hex_search(needle);
    win->hex_view->set_matches(res);
    update_find_info();
    if (!res.empty()) win->hex_view->find_next();
    update_status();
}

// ============================================================
// Окно поиска
// ============================================================
void update_find_info() {
    if (!find_info) return;
    char b[96];
    if (win->mode == Mode::TEXT) {
        if (text_search.matches.empty()) b[0] = 0;
        else {
            int cur = text_search.current >= 0 ? text_search.current + 1 : 0;
            std::snprintf(b, sizeof(b), "%d / %d", cur, (int)text_search.matches.size());
        }
    } else {
        int total = win->hex_view ? win->hex_view->match_count() : 0;
        int cur   = win->hex_view ? win->hex_view->current_match_index() : -1;
        if (total == 0) std::snprintf(b, sizeof(b), "0 / 0");
        else std::snprintf(b, sizeof(b), "%d / %d", cur + 1, total);
    }
    find_info->copy_label(b);
    find_info->redraw();
}

void on_find_next();
void on_find_prev();

void create_find_win() {
    const int FW = 620, FH = 160;
    find_win = new Fl_Window(FW, FH, "Поиск");
    find_win->set_non_modal();
    find_win->color(DLG_BG);
    find_win->begin();

    Fl_Box* lbl1 = new Fl_Box(10, 12, 90, 25, "Найти:");
    lbl1->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    lbl1->labelcolor(DLG_FG);
    find_input = new Fl_Input(105, 12, 380, 25);
    find_input->color(fl_rgb_color(20, 24, 28));
    find_input->textcolor(DLG_FG);
    find_input->textsize(13);

    find_match_case = new Fl_Check_Button(495, 12, 55, 25, "Aa");
    find_match_case->labelcolor(DLG_FG);
    find_match_case->selection_color(COLOR_SELECTION);

    find_as_hex = new Fl_Check_Button(545, 12, 65, 25, "Hex");
    find_as_hex->labelcolor(DLG_FG);
    find_as_hex->value(1);

    replace_lbl = new Fl_Box(10, 44, 90, 25, "Заменить:");
    replace_lbl->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);
    replace_lbl->labelcolor(DLG_FG);
    replace_input = new Fl_Input(105, 44, 380, 25);
    replace_input->color(fl_rgb_color(20, 24, 28));
    replace_input->textcolor(DLG_FG);
    replace_input->textsize(13);

    find_info = new Fl_Box(105, 76, 380, 22, "");
    find_info->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    find_info->labelsize(12);
    find_info->labelcolor(COLOR_STATUS_ACC);

    Fl_Button* b_prev  = new Fl_Button(105, 105, 42, 30, "@<");
    Fl_Button* b_next  = new Fl_Button(153, 105, 42, 30, "@>");
    b_replace     = new Fl_Button(210, 105, 110, 30, "Заменить");
    b_replace_all = new Fl_Button(325, 105, 140, 30, "Заменить всё");
    Fl_Button* b_close = new Fl_Button(490, 105, 100, 30, "Закрыть");

    for (Fl_Widget* w : { (Fl_Widget*)b_prev, (Fl_Widget*)b_next,
                          (Fl_Widget*)b_replace, (Fl_Widget*)b_replace_all,
                          (Fl_Widget*)b_close }) {
        w->color(DLG_BTN);
        w->labelcolor(DLG_FG);
        w->box(FL_FLAT_BOX);
    }

    find_win->end();

    find_input->when(FL_WHEN_ENTER_KEY);
    find_input->callback([](Fl_Widget*, void*) {
        if (Fl::event_key() == FL_Enter || Fl::event_key() == FL_KP_Enter)
            on_find_next();
    });
    find_match_case->callback([](Fl_Widget*, void*) {
        text_search.dirty = true;
        if (win->mode == Mode::TEXT) on_find_next();
    });
    find_as_hex->callback([](Fl_Widget*, void*) {
        if (win->mode == Mode::HEX) { hex_search_term.clear(); on_find_next(); }
    });
    b_next->callback([](Fl_Widget*, void*) { on_find_next(); });
    b_prev->callback([](Fl_Widget*, void*) { on_find_prev(); });
    b_replace->callback([](Fl_Widget*, void*) {
        if (win->mode == Mode::TEXT) text_replace_current();
    });
    b_replace_all->callback([](Fl_Widget*, void*) {
        if (win->mode == Mode::TEXT) text_replace_all();
    });
    b_close->callback([](Fl_Widget*, void*) {
        find_win->hide();
        if (win->mode == Mode::HEX) win->hex_view->clear_matches();
        else                        tbuf->unselect();
        tbuf->unhighlight();
        text_search.matches.clear();
        text_search.current = -1;
        text_search.dirty = true;
        update_find_info();
        update_status();
        if (win->mode == Mode::HEX) win->hex_view->take_focus();
        else                        win->text_ed->take_focus();
    });
}

void open_find(bool focus_replace) {
    if (!find_win) create_find_win();
    bool is_hex = (win->mode == Mode::HEX);
    find_win->copy_label(is_hex ? "Поиск в HEX" : "Поиск в тексте");
    if (is_hex) {
        find_as_hex->show();
        replace_lbl->deactivate();
        replace_input->deactivate();
        b_replace->deactivate();
        b_replace_all->deactivate();
    } else {
        find_as_hex->hide();
        replace_lbl->activate();
        replace_input->activate();
        b_replace->activate();
        b_replace_all->activate();
    }
    find_win->show();
    if (focus_replace && !is_hex) {
        replace_input->take_focus();
    } else {
        find_input->take_focus();
        find_input->insert_position(find_input->size());
    }
    update_find_info();
}

void on_find_next() {
    if (win->mode == Mode::TEXT) {
        text_find_next(true);
    } else {
        std::string term = find_input->value() ? find_input->value() : "";
        bool as_hex = find_as_hex->value();
        if (term != hex_search_term || as_hex != hex_search_as_hex) {
            hex_search_term = term;
            hex_search_as_hex = as_hex;
            run_hex_search();
        } else {
            win->hex_view->find_next();
            update_find_info();
            update_status();
        }
    }
}

void on_find_prev() {
    if (win->mode == Mode::TEXT) {
        text_find_next(false);
    } else {
        std::string term = find_input->value() ? find_input->value() : "";
        bool as_hex = find_as_hex->value();
        if (term != hex_search_term || as_hex != hex_search_as_hex) {
            hex_search_term = term;
            hex_search_as_hex = as_hex;
            run_hex_search();
            win->hex_view->find_prev();
        } else {
            win->hex_view->find_prev();
        }
        update_find_info();
        update_status();
    }
}

// ============================================================
// Нативные диалоги файлов
// ============================================================
std::string choose_open_file() {
    Fl_Native_File_Chooser fc;
    fc.title("Открыть файл");
    fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
    fc.filter("Все файлы\t*\n"
              "Текстовые (*.txt)\t*.txt\n"
              "Исходники (*.c, *.cpp, *.h, *.py, *.rs, *.go, *.js, *.ts)\t*.c;*.cpp;*.h;*.hpp;*.py;*.rs;*.go;*.js;*.ts;*.java;*.cs\n"
              "Веб (*.html, *.css, *.json, *.xml)\t*.html;*.htm;*.css;*.json;*.xml;*.svg\n"
              "Markdown (*.md)\t*.md\n"
              "Конфиги (*.ini, *.yaml, *.toml)\t*.ini;*.yaml;*.yml;*.toml");
    if (fc.show() == 0 && fc.filename()) return fc.filename();
    return "";
}

std::string choose_save_file(const std::string& suggested) {
    Fl_Native_File_Chooser fc;
    fc.title("Сохранить как");
    fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
    fc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM);
    std::string name = suggested.empty() ? "untitled" : basename_of(suggested);
    fc.preset_file(name.c_str());
    if (fc.show() == 0 && fc.filename()) return fc.filename();
    return "";
}

// ============================================================
// Определение, бинарный ли файл
// ============================================================
bool looks_binary(const uint8_t* data, size_t len) {
    if (!data || len == 0) return false;
    const size_t scan = std::min<size_t>(len, 8192);
    size_t bad = 0;
    for (size_t i = 0; i < scan; ++i) {
        uint8_t b = data[i];
        if (b == 0) return true;
        if (b < 32 && b != 9 && b != 10 && b != 13) bad++;
    }
    return (bad * 100 / scan) > 5;
}

// ============================================================
// Авто-ширина жёлоба с номерами строк
// ============================================================
static bool g_show_linenumbers  = true;
static bool g_word_wrap         = false;

static void update_gutter_width() {
    if (!win || !win->text_ed || !tbuf) return;
    if (!g_show_linenumbers) return;

    int lines = tbuf->count_lines(0, tbuf->length()) + 1;
    if (lines < 1) lines = 1;
    int digits = 1;
    int tmp = lines;
    while (tmp >= 10) { tmp /= 10; digits++; }
    if (digits < 4) digits = 4;
    if (digits > 9) digits = 9;

    fl_font(FL_FREE_FONT, text_size);
    int char_w = (int)fl_width('0');
    if (char_w < 1) char_w = text_size / 2;

    int width = char_w * digits + 16;
    if (width < 40)  width = 40;
    if (width > 160) width = 160;

    if (win->text_ed->linenumber_width() != width)
        win->text_ed->linenumber_width(width);
}

static void apply_editor_settings() {
    if (!win || !win->text_ed) return;

    if (g_show_linenumbers) {
        if (win->text_ed->linenumber_width() == 0)
            win->text_ed->linenumber_width(GUTTER_W);
        update_gutter_width();
    } else {
        win->text_ed->linenumber_width(0);
    }

    if (g_word_wrap) win->text_ed->wrap_mode(Fl_Text_Display::WRAP_AT_PIXEL, 0);
    else             win->text_ed->wrap_mode(Fl_Text_Display::WRAP_NONE, 0);

    win->text_ed->redraw();
}

// ============================================================
// Файловые операции
// ============================================================
bool save_current(bool force_as = false) {
    if (win->mode == Mode::TEXT) {
        std::string path = current_path;
        if (path.empty() || force_as) {
            path = choose_save_file(current_path);
            if (path.empty()) return false;
        }

        std::string utf8;
        int tlen = tbuf->length();
        if (tlen > 0) {
            char* raw = tbuf->text_range(0, tlen);
            if (raw) { utf8.assign(raw, (size_t)tlen); free(raw); }
        }

        std::string with_eol = apply_eol(utf8, g_text_info.eol);
        std::string bytes;
        if (!encode_from_utf8(with_eol, g_text_info.enc, bytes)) {
            TeditDialog::alert("Ошибка", "Не удалось преобразовать текст в исходную кодировку.");
            return false;
        }

        hdoc.file.close();

        if (!write_file_atomic(path, bytes)) {
            hdoc.file.open(path);
            TeditDialog::alert("Ошибка", ("Не удалось сохранить:\n" + path).c_str());
            return false;
        }

        hdoc.file.open(path);
        hdoc.edits.clear();
        hdoc.undo_stack.clear();
        hdoc.redo_stack.clear();
        dirty_hex = false;

        current_path = path;
        dirty_text = false;
        win->hex_view->set_doc(&hdoc);
        update_title(); update_status();
        if (prefs) prefs->set("last_file", path.c_str());
        return true;
    } else {
        if (!hdoc.is_open()) return false;
        if (!hdoc.has_edits()) { dirty_hex = false; update_title(); return true; }
        std::string path = current_path;
        if (path.empty() || force_as) {
            path = choose_save_file(current_path);
            if (path.empty()) return false;
        }
        if (!hdoc.save(path)) {
            TeditDialog::alert("Ошибка", "Не удалось сохранить hex-правки.");
            return false;
        }
        current_path = path; dirty_hex = false;
        win->hex_view->set_doc(&hdoc);
        update_title(); update_status();
        if (prefs) prefs->set("last_file", path.c_str());
        return true;
    }
}

bool maybe_save() {
    if (!is_dirty()) return true;
    int r = TeditDialog::confirm("Несохранённые изменения",
                                 "Файл был изменён.\nСохранить перед продолжением?",
                                 "Сохранить", "Не сохранять");
    if (r == 0) return save_current();
    return true;
}

void open_file(const std::string& path, bool interactive = true) {
    if (!hdoc.file.open(path)) {
        TeditDialog::alert("Ошибка", ("Не удалось открыть:\n" + path).c_str());
        return;
    }
    hdoc.edits.clear(); hdoc.undo_stack.clear(); hdoc.redo_stack.clear();
    dirty_hex = false;

    text_search.matches.clear();
    text_search.current = -1;
    text_search.dirty = true;
    if (tbuf) tbuf->unhighlight();

    bool binary = looks_binary(hdoc.file.ptr(), hdoc.length());
    bool too_big = hdoc.length() > TEXT_MODE_LIMIT;

    Mode target_mode = Mode::TEXT;
    if (too_big) {
        target_mode = Mode::HEX;
        if (interactive)
            TeditDialog::info("Большой файл",
                "Файл слишком большой для текстового режима.\nОткрыт в HEX.");
    } else if (binary) {
        if (interactive) {
            int r = TeditDialog::confirm("Похоже на бинарный файл",
                "Файл содержит двоичные данные.\n\n"
                "Открыть в HEX-режиме? Текстовый режим покажет\n"
                "искажённое содержимое.",
                "Да, в HEX", "Нет, в текст");
            target_mode = (r == 0) ? Mode::HEX : Mode::TEXT;
        } else {
            target_mode = Mode::HEX;
        }
    }

    if (target_mode == Mode::TEXT) {
        g_text_info = detect_text_info(hdoc.file.ptr(), hdoc.length());
        size_t skip = (size_t)enc_bom_len(g_text_info.enc);
        std::string decoded;
        if (!decode_to_utf8(hdoc.file.ptr() + skip, hdoc.length() - skip, g_text_info.enc, decoded)) {
            decoded.assign((const char*)hdoc.file.ptr(), hdoc.length());
        }
        std::string normalized = normalize_eol_to_lf(decoded);
        tbuf->text(normalized.c_str());

        dirty_text = false;
        win->text_ed->insert_position(0);

        g_lang = detect_language(path, tbuf->address(0), tbuf->length());
        if (g_lang != LangId::None) {
            apply_syntax_highlight();
        } else {
            if (g_style_buf) g_style_buf->text("");
        }
        update_gutter_width();
    } else {
        tbuf->text("");
        dirty_text = false;
        if (g_style_buf) g_style_buf->text("");
        g_lang = LangId::None;
    }

    current_path = path;
    win->hex_view->set_doc(&hdoc);
    win->set_mode(target_mode);

    update_title(); update_status();
    if (prefs) prefs->set("last_file", path.c_str());
}

// ============================================================
// Снэпшоты (локальные бэкапы рядом с файлом)
// ============================================================
static std::string snapshot_dir(const std::string& file_path) {
    size_t slash = file_path.find_last_of("/\\");
    std::string dir = (slash == std::string::npos) ? "" : file_path.substr(0, slash);
    return dir + "\\.tedit_snaps";
}

static std::string snapshot_filename(const std::string& file_path) {
    size_t slash = file_path.find_last_of("/\\");
    std::string bn = (slash == std::string::npos) ? file_path : file_path.substr(slash + 1);
    SYSTEMTIME st;
    GetLocalTime(&st);
    char buf[512];
    std::snprintf(buf, sizeof(buf), "%s.%04d%02d%02d-%02d%02d%02d-%03d",
                  bn.c_str(),
                  st.wYear, st.wMonth, st.wDay,
                  st.wHour, st.wMinute, st.wSecond,
                  st.wMilliseconds);
    return buf;
}

static bool get_current_content_bytes(std::string& out) {
    out.clear();
    if (win->mode == Mode::TEXT) {
        std::string utf8;
        int tlen = tbuf->length();
        if (tlen > 0) {
            char* raw = tbuf->text_range(0, tlen);
            if (raw) { utf8.assign(raw, (size_t)tlen); free(raw); }
        }
        std::string with_eol = apply_eol(utf8, g_text_info.eol);
        return encode_from_utf8(with_eol, g_text_info.enc, out);
    } else {
        if (!hdoc.is_open()) return false;
        size_t len = hdoc.length();
        out.resize(len);
        for (size_t i = 0; i < len; ++i) out[i] = (char)hdoc.get(i);
        return true;
    }
}

static bool create_snapshot_impl(std::string* created_path = nullptr) {
    if (current_path.empty()) {
        TeditDialog::alert("Снэпшот",
            "Сначала сохраните файл — снэпшоты создаются\n"
            "в папке .tedit_snaps рядом с ним.");
        return false;
    }

    std::string dir = snapshot_dir(current_path);
    if (!CreateDirectoryA(dir.c_str(), nullptr)) {
        DWORD e = GetLastError();
        if (e != ERROR_ALREADY_EXISTS) {
            TeditDialog::alert("Снэпшот", "Не удалось создать папку .tedit_snaps.");
            return false;
        }
    }

    std::string content;
    if (!get_current_content_bytes(content)) {
        TeditDialog::alert("Снэпшот", "Не удалось получить содержимое для снэпшота.");
        return false;
    }

    std::string fname = snapshot_filename(current_path);
    std::string full  = dir + "\\" + fname;
    int n = 1;
    while (GetFileAttributesA(full.c_str()) != INVALID_FILE_ATTRIBUTES) {
        char sfx[16];
        std::snprintf(sfx, sizeof(sfx), "-%d", n++);
        full = dir + "\\" + fname + sfx;
    }

    std::ofstream o(full, std::ios::binary | std::ios::trunc);
    if (!o) {
        TeditDialog::alert("Снэпшот", "Не удалось записать снэпшот.");
        return false;
    }
    o.write(content.data(), (std::streamsize)content.size());
    o.close();
    if (!o) {
        std::remove(full.c_str());
        TeditDialog::alert("Снэпшот", "Ошибка записи снэпшота.");
        return false;
    }

    if (created_path) *created_path = full;
    return true;
}

struct SnapshotEntry {
    std::string filename;
    std::string full_path;
    uint64_t    size = 0;
    FILETIME    ft{};
};

static void list_snapshots(const std::string& dir, std::vector<SnapshotEntry>& out) {
    out.clear();
    WIN32_FIND_DATAA fd;
    std::string mask = dir + "\\*";
    HANDLE h = FindFirstFileA(mask.c_str(), &fd);
    if (h == INVALID_HANDLE_VALUE) return;
    do {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) continue;
        SnapshotEntry e;
        e.filename  = fd.cFileName;
        e.full_path = dir + "\\" + fd.cFileName;
        e.size      = ((uint64_t)fd.nFileSizeHigh << 32) | fd.nFileSizeLow;
        e.ft        = fd.ftLastWriteTime;
        out.push_back(std::move(e));
    } while (FindNextFileA(h, &fd));
    FindClose(h);
    std::sort(out.begin(), out.end(),
              [](const SnapshotEntry& a, const SnapshotEntry& b){
                  return CompareFileTime(&a.ft, &b.ft) > 0;
              });
}

static bool restore_snapshot(const std::string& snap_path) {
    std::ifstream in(snap_path, std::ios::binary);
    if (!in) return false;
    in.seekg(0, std::ios::end);
    std::streamoff sz = in.tellg();
    in.seekg(0, std::ios::beg);
    std::string bytes;
    bytes.resize((size_t)sz);
    if (sz > 0) in.read(&bytes[0], sz);
    in.close();

    hdoc.file.close();
    if (!write_file_atomic(current_path, bytes)) {
        hdoc.file.open(current_path);
        return false;
    }
    hdoc.file.open(current_path);
    hdoc.edits.clear();
    hdoc.undo_stack.clear();
    hdoc.redo_stack.clear();
    dirty_hex = false;

    if (win->mode == Mode::TEXT) {
        g_text_info = detect_text_info(hdoc.file.ptr(), hdoc.length());
        size_t skip = (size_t)enc_bom_len(g_text_info.enc);
        std::string decoded;
        if (!decode_to_utf8(hdoc.file.ptr() + skip, hdoc.length() - skip, g_text_info.enc, decoded)) {
            decoded.assign((const char*)hdoc.file.ptr(), hdoc.length());
        }
        std::string normalized = normalize_eol_to_lf(decoded);
        tbuf->text(normalized.c_str());
        dirty_text = false;
        win->text_ed->insert_position(0);
        g_lang = detect_language(current_path, tbuf->address(0), tbuf->length());
        if (g_lang != LangId::None) apply_syntax_highlight();
        else if (g_style_buf) g_style_buf->text("");
        update_gutter_width();
    } else {
        tbuf->text("");
        if (g_style_buf) g_style_buf->text("");
        win->hex_view->set_doc(&hdoc);
    }

    update_title();
    update_status();
    return true;
}

// ============================================================
// Состояние
// ============================================================

void apply_text_size(int s);

void save_state() {
    if (!prefs || !win) return;
    prefs->set("window_x", win->x());
    prefs->set("window_y", win->y());
    prefs->set("window_w", win->w());
    prefs->set("window_h", win->h());
    prefs->set("text_size", text_size);
    prefs->set("mode", win->mode == Mode::HEX ? 1 : 0);
    prefs->set("find_match_case", find_match_case ? find_match_case->value() : 0);
    prefs->set("find_as_hex", find_as_hex ? find_as_hex->value() : 1);
    prefs->set("last_file", current_path.c_str());

    if (win->mode == Mode::TEXT && win->text_ed) {
        prefs->set("last_cursor_pos", win->text_ed->insert_position());
    } else if (win->mode == Mode::HEX && win->hex_view && hdoc.is_open()) {
        prefs->set("last_hex_top",    (int)win->hex_view->top_);
        prefs->set("last_hex_cursor", (int)win->hex_view->cursor_offset());
        prefs->set("last_hex_ascii",  win->hex_view->in_ascii_ ? 1 : 0);
    }
    prefs->set("show_linenumbers", g_show_linenumbers ? 1 : 0);
    prefs->set("word_wrap",        g_word_wrap ? 1 : 0);
    prefs->set("theme",            (int)g_theme);
    prefs->flush();
}

void load_state() {
    if (!prefs || !win) return;
    int x = 100, y = 100, w = INIT_W, h = INIT_H;
    prefs->get("window_x", x, 100);
    prefs->get("window_y", y, 100);
    prefs->get("window_w", w, INIT_W);
    prefs->get("window_h", h, INIT_H);
    if (w < 400) w = INIT_W;
    if (h < 300) h = INIT_H;
    win->resize(x, y, w, h);
    win->layout();

    int ts = TEXT_SIZE_DEF;
    prefs->get("text_size", ts, TEXT_SIZE_DEF);
    apply_text_size(ts);

    int m = 0;
    prefs->get("mode", m, 0);
    win->set_mode(m == 1 ? Mode::HEX : Mode::TEXT);

    int mc = 0, ah = 1;
    prefs->get("find_match_case", mc, 0);
    prefs->get("find_as_hex", ah, 1);
    if (find_match_case) find_match_case->value(mc);
    if (find_as_hex)     find_as_hex->value(ah);
    hex_search_as_hex = (ah != 0);

    // Настройки редактора
    int sl = 1;
    prefs->get("show_linenumbers", sl, 1);
    g_show_linenumbers = (sl != 0);
    int ww = 0;
    prefs->get("word_wrap", ww, 0);
    g_word_wrap = (ww != 0);

    apply_editor_settings();

    // Синхронизируем состояние галочек в меню
    if (win->menubar) {
        auto set_flag = [](Fl_Menu_Bar* mb, const char* path, bool on) {
            Fl_Menu_Item* mi = const_cast<Fl_Menu_Item*>(mb->find_item(path));
            if (!mi) return;
            if (on) mi->set();
            else    mi->clear();
        };

        set_flag(win->menubar, "&Вид/&Номера строк",  g_show_linenumbers);
        set_flag(win->menubar, "&Вид/&Перенос строк", g_word_wrap);
    }

    int th = 0;
    prefs->get("theme", th, 0);
    if (th < 0 || th >= (int)ThemeId::Count) th = 0;
    apply_theme((ThemeId)th);
}

// ============================================================
// Меню
// ============================================================
static void show_snapshot_dialog() {
    if (current_path.empty()) {
        TeditDialog::alert("Снэпшоты", "Сначала сохраните файл.");
        return;
    }
    std::string dir = snapshot_dir(current_path);
    CreateDirectoryA(dir.c_str(), nullptr);
    std::vector<SnapshotEntry> entries;
    list_snapshots(dir, entries);

    const int W = 760, H = 480;
    Fl_Window* dlg = new Fl_Window(W, H, "История снэпшотов");
    dlg->set_modal();
    dlg->color(DLG_BG);
    dlg->begin();

    Fl_Box* tb = new Fl_Box(0, 0, W, 32);
    tb->box(FL_FLAT_BOX);
    tb->color(DLG_TITLE_BG);
    tb->labelcolor(DLG_FG);
    tb->labelsize(14);
    tb->labelfont(FL_HELVETICA_BOLD);
    tb->copy_label("История снэпшотов");
    tb->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

    Fl_Hold_Browser* br = new Fl_Hold_Browser(12, 44, W - 24, H - 44 - 64);
    br->color(fl_rgb_color(20, 24, 28));
    br->textcolor(DLG_FG);
    br->textsize(13);
    br->selection_color(COLOR_SELECTION);
    br->has_scrollbar(Fl_Browser_::VERTICAL);

    for (auto& e : entries) {
        FILETIME ft_local;
        FileTimeToLocalFileTime(&e.ft, &ft_local);
        SYSTEMTIME st;
        FileTimeToSystemTime(&ft_local, &st);
        char size_str[32];
        if (e.size < 1024)
            std::snprintf(size_str, sizeof(size_str), "%llu B", (unsigned long long)e.size);
        else if (e.size < 1024ull * 1024)
            std::snprintf(size_str, sizeof(size_str), "%.1f KB", e.size / 1024.0);
        else
            std::snprintf(size_str, sizeof(size_str), "%.2f MB", e.size / 1024.0 / 1024.0);
        char line[512];
        std::snprintf(line, sizeof(line), "%04d-%02d-%02d %02d:%02d:%02d    %8s    %s",
                      st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond,
                      size_str, e.filename.c_str());
        br->add(line);
    }
    if (br->size() > 0) br->select(1);

    Fl_Box* info = new Fl_Box(12, H - 56, 300, 24);
    info->labelcolor(DLG_FG);
    info->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
    {
        char b[64];
        std::snprintf(b, sizeof(b), "Всего: %d", (int)entries.size());
        info->copy_label(b);
    }

    int bx = W - 12 - (140 * 4 + 10 * 3);
    Fl_Button* b_restore = new Fl_Button(bx, H - 48, 140, 32, "Восстановить"); bx += 150;
    Fl_Button* b_delete  = new Fl_Button(bx, H - 48, 140, 32, "Удалить");      bx += 150;
    Fl_Button* b_folder  = new Fl_Button(bx, H - 48, 140, 32, "Открыть папку"); bx += 150;
    Fl_Button* b_close   = new Fl_Button(bx, H - 48, 140, 32, "Закрыть");
    for (Fl_Widget* w : { (Fl_Widget*)b_restore, (Fl_Widget*)b_delete,
                          (Fl_Widget*)b_folder,  (Fl_Widget*)b_close }) {
        w->color(DLG_BTN);
        w->labelcolor(DLG_FG);
        w->box(FL_FLAT_BOX);
    }

    struct Ctx { Fl_Window* dlg; Fl_Hold_Browser* br; std::vector<SnapshotEntry>* entries; std::string dir; };
    Ctx* ctx = new Ctx{ dlg, br, &entries, dir };

    b_close->callback([](Fl_Widget*, void* d){ ((Ctx*)d)->dlg->hide(); }, ctx);

    b_folder->callback([](Fl_Widget*, void* d) {
        Ctx* c = (Ctx*)d;
        ShellExecuteA(nullptr, "open", c->dir.c_str(), nullptr, nullptr, SW_SHOW);
    }, ctx);

    b_restore->callback([](Fl_Widget*, void* d) {
        Ctx* c = (Ctx*)d;
        int idx = c->br->value();
        if (idx < 1 || idx > (int)c->entries->size()) return;
        auto& e = (*c->entries)[idx - 1];
        int r = TeditDialog::confirm("Восстановить снэпшот",
            "Текущий файл будет заменён выбранным снэпшотом.\n\n"
            "Несохранённые правки в текущем буфере будут потеряны.\n"
            "Если хотите их сохранить — сначала нажмите Ctrl+Alt+S.",
            "Восстановить", "Отмена");
        if (r != 0) return;
        if (!restore_snapshot(e.full_path)) {
            TeditDialog::alert("Ошибка", "Не удалось восстановить снэпшот.");
            return;
        }
        c->dlg->hide();
    }, ctx);

    b_delete->callback([](Fl_Widget*, void* d) {
        Ctx* c = (Ctx*)d;
        int idx = c->br->value();
        if (idx < 1 || idx > (int)c->entries->size()) return;
        auto& e = (*c->entries)[idx - 1];
        std::string q = "Удалить снэпшот?\n\n" + e.filename;
        int r = TeditDialog::confirm("Удалить снэпшот", q.c_str(), "Удалить", "Отмена");
        if (r != 0) return;
        std::remove(e.full_path.c_str());
        c->entries->erase(c->entries->begin() + (idx - 1));
        c->br->remove(idx);
        if (c->br->size() > 0) {
            int n = std::min(idx, c->br->size());
            c->br->select(n);
        }
    }, ctx);

    dlg->end();
    dlg->callback([](Fl_Widget*, void* d){ ((Ctx*)d)->dlg->hide(); }, ctx);
    dlg->position(Fl::w() / 2 - W / 2, Fl::h() / 2 - H / 2);
    dlg->show();
    while (dlg->shown()) Fl::wait();
    delete ctx;
    Fl::delete_widget(dlg);
}

void menu_snapshot_create(Fl_Widget*, void*) {
    std::string created;
    if (create_snapshot_impl(&created)) {
        set_flash("Снэпшот: " + basename_of(created));
        update_status();
    }
}

void menu_snapshot_history(Fl_Widget*, void*) {
    show_snapshot_dialog();
}

void menu_snapshot_folder(Fl_Widget*, void*) {
    if (current_path.empty()) {
        TeditDialog::alert("Снэпшоты", "Сначала сохраните файл.");
        return;
    }
    std::string dir = snapshot_dir(current_path);
    CreateDirectoryA(dir.c_str(), nullptr);
    ShellExecuteA(nullptr, "open", dir.c_str(), nullptr, nullptr, SW_SHOW);
}

void menu_new(Fl_Widget*, void*) {
    if (!maybe_save()) return;
    hdoc.reset();
    tbuf->text("");
    tbuf->unhighlight();
    current_path.clear();
    dirty_text = dirty_hex = false;
    text_search.matches.clear();
    text_search.current = -1;
    text_search.dirty = true;
    hex_search_term.clear();
    g_lang = LangId::None;
    g_text_info = TextFileInfo{};
    if (g_style_buf) g_style_buf->text("");
    win->set_mode(Mode::TEXT);
    if (find_win && find_win->visible()) { win->hex_view->clear_matches(); update_find_info(); }
    update_gutter_width();
    update_title(); update_status();
}
void menu_open(Fl_Widget*, void*) {
    if (!maybe_save()) return;
    std::string path = choose_open_file();
    if (!path.empty()) open_file(path);
}
void menu_save(Fl_Widget*, void*)    { save_current(false); }
void menu_save_as(Fl_Widget*, void*) { save_current(true); }
void menu_quit(Fl_Widget*, void*)    { if (maybe_save()) win->hide(); }

void menu_mode_text(Fl_Widget*, void*) {
    if (hdoc.length() > TEXT_MODE_LIMIT && hdoc.is_open()) {
        TeditDialog::alert("Режим недоступен",
            "Файл слишком большой для текстового режима.\nИспользуйте HEX.");
        return;
    }
    if (find_win && find_win->visible()) find_win->hide();
    win->set_mode(Mode::TEXT);
    if (g_lang == LangId::None && !current_path.empty())
        g_lang = detect_language(current_path, tbuf->address(0), tbuf->length());
    if (g_lang != LangId::None) apply_syntax_highlight();
    update_status();
}
void menu_mode_hex(Fl_Widget*, void*) {
    if (find_win && find_win->visible()) find_win->hide();
    win->set_mode(Mode::HEX);
    update_status();
}
void menu_mode_toggle(Fl_Widget*, void*) {
    if (win->mode == Mode::TEXT) menu_mode_hex(nullptr,nullptr);
    else                          menu_mode_text(nullptr,nullptr);
}

void menu_toggle_linenumbers(Fl_Widget* w, void*) {
    g_show_linenumbers = !g_show_linenumbers;
    Fl_Menu_Bar* mb = (Fl_Menu_Bar*)w;
    const Fl_Menu_Item* mi = mb ? mb->mvalue() : nullptr;
    if (mi) {
        Fl_Menu_Item* mmi = const_cast<Fl_Menu_Item*>(mi);
        if (g_show_linenumbers) mmi->set();
        else                    mmi->clear();
    }
    apply_editor_settings();
}

void menu_toggle_wordwrap(Fl_Widget* w, void*) {
    g_word_wrap = !g_word_wrap;
    Fl_Menu_Bar* mb = (Fl_Menu_Bar*)w;
    const Fl_Menu_Item* mi = mb ? mb->mvalue() : nullptr;
    if (mi) {
        Fl_Menu_Item* mmi = const_cast<Fl_Menu_Item*>(mi);
        if (g_word_wrap) mmi->set();
        else             mmi->clear();
    }
    apply_editor_settings();
}

void apply_text_size(int s) {
    text_size = std::max(8, std::min(48, s));
    if (win && win->text_ed) {
        win->text_ed->textsize(text_size);
        win->text_ed->linenumber_size(text_size);
        for (int i = 0; i < G_STYLE_COUNT; ++i) g_style_table[i].size = text_size;
        if (g_syntax_dirty) schedule_syntax_highlight();
        else if (g_lang != LangId::None) apply_syntax_highlight();
        update_gutter_width();
        apply_editor_settings();
        win->text_ed->redraw();
    }
}
void menu_zoom_in(Fl_Widget*, void*)    { apply_text_size(text_size + 2); }
void menu_zoom_out(Fl_Widget*, void*)   { apply_text_size(text_size - 2); }
void menu_zoom_reset(Fl_Widget*, void*) { apply_text_size(TEXT_SIZE_DEF); }

void menu_copy(Fl_Widget*, void*) {
    if (win->mode == Mode::HEX) {
        if (win->hex_view->has_selection()) win->hex_view->copy_selection(true);
    } else {
        int s = 0, e = 0;
        if (tbuf->selection_position(&s, &e) && s < e) {
            char* txt = tbuf->text_range(s, e);
            if (txt) { clipboard_set_utf8(std::string(txt)); free(txt); }
        }
    }
}
void menu_copy_ascii(Fl_Widget*, void*) {
    if (win->mode == Mode::HEX) {
        if (win->hex_view->has_selection()) win->hex_view->copy_selection(false);
    } else {
        menu_copy(nullptr, nullptr);
    }
}
void menu_cut(Fl_Widget*, void*) {
    if (win->mode == Mode::HEX) {
        if (!win->hex_view->has_selection()) return;
        win->hex_view->copy_selection(true);
        size_t a = win->hex_view->sel_start_;
        size_t b = win->hex_view->sel_end_;
        if (a < b) {
            std::vector<uint8_t> zeros(b - a, 0);
            hdoc.set_bytes(a, zeros);
        }
        win->hex_view->clear_selection();
        dirty_hex = !hdoc.edits.empty();
        win->hex_view->redraw();
        update_title();
        update_status();
    } else {
        int s = 0, e = 0;
        if (!tbuf->selection_position(&s, &e) || s >= e) return;
        char* txt = tbuf->text_range(s, e);
        if (!txt) return;
        clipboard_set_utf8(std::string(txt));
        free(txt);
        tbuf->replace(s, e, "");
        win->text_ed->insert_position(s);
        win->text_ed->show_insert_position();
        text_search.dirty = true;
    }
}
void menu_paste(Fl_Widget*, void*) {
    if (win->mode == Mode::HEX) {
        win->hex_view->paste_hex();
    } else {
        std::string txt = clipboard_get_utf8();
        if (txt.empty()) return;
        int s = 0, e = 0;
        bool has_sel = tbuf->selection_position(&s, &e) && s < e;
        if (has_sel) {
            tbuf->replace(s, e, txt.c_str());
            win->text_ed->insert_position(s + (int)txt.size());
        } else {
            int pos = win->text_ed->insert_position();
            tbuf->insert(pos, txt.c_str());
            win->text_ed->insert_position(pos + (int)txt.size());
        }
        win->text_ed->show_insert_position();
        text_search.dirty = true;
    }
}
void menu_select_all(Fl_Widget*, void*) {
    if (win->mode == Mode::HEX) {
        win->hex_view->select_all();
    } else {
        tbuf->select(0, tbuf->length());
        win->text_ed->take_focus();
    }
}

void menu_undo(Fl_Widget*, void*) {
    if (win->mode == Mode::HEX) {
        if (hdoc.undo_last()) {
            dirty_hex = !hdoc.edits.empty();
            win->hex_view->redraw();
            update_title(); update_status();
        }
    } else {
        if (tbuf->undo()) {
            text_search.dirty = true;
            schedule_syntax_highlight();
            update_status();
        }
    }
}
void menu_redo(Fl_Widget*, void*) {
    if (win->mode == Mode::HEX) {
        if (hdoc.redo_last()) {
            dirty_hex = !hdoc.edits.empty();
            win->hex_view->redraw();
            update_title(); update_status();
        }
    } else {
        if (tbuf->redo()) {
            text_search.dirty = true;
            schedule_syntax_highlight();
            update_status();
        }
    }
}

void menu_goto(Fl_Widget*, void*) {
    if (win->mode == Mode::HEX && hdoc.is_open()) {
        std::string s;
        if (TeditDialog::input("Перейти к offset",
                "Введите hex-offset (напр. 0x1A3F или 1A3F):",
                s, "0") != 0) return;
        if (s.empty()) return;
        size_t off = (size_t)strtoull(s.c_str(), nullptr, 16);
        if (off >= hdoc.length()) off = hdoc.length() - 1;
        win->hex_view->goto_offset(off);
        win->hex_view->take_focus();
        update_status();
    } else if (win->mode == Mode::TEXT) {
        std::string s;
        if (TeditDialog::input("Перейти к строке", "Введите номер строки:", s, "1") != 0) return;
        long line = strtol(s.c_str(), nullptr, 10);
        if (line < 1) line = 1;
        int pos = tbuf->skip_lines(0, (int)line - 1);
        if (pos < 0) pos = tbuf->length();
        win->text_ed->insert_position(pos);
        win->text_ed->show_insert_position();
        win->text_ed->take_focus();
        update_status();
    }
}

void menu_find(Fl_Widget*, void*)      { open_find(false); }
void menu_find_repl(Fl_Widget*, void*) { open_find(true); }
void menu_find_next(Fl_Widget*, void*) {
    if (!find_win || !find_input || !find_input->value() || !find_input->value()[0]) {
        open_find(false); return;
    }
    on_find_next();
}
void menu_find_prev(Fl_Widget*, void*) {
    if (!find_win || !find_input || !find_input->value() || !find_input->value()[0]) {
        open_find(false); return;
    }
    on_find_prev();
}

void win_close_cb(Fl_Widget* w, void*) {
    if (!maybe_save()) return;
    save_state();
    if (win->text_ed) win->text_ed->stop_calm_cursor();
    Fl::remove_timeout(status_tick);
    Fl::remove_timeout(syntax_debounce_cb);
    w->hide();
}

void on_text_modify(int pos, int ins, int del, int, const char*, void*) {
    if (ins == 0 && del == 0) return;
    if (!dirty_text) { dirty_text = true; update_title(); }
    update_status();
    text_search.dirty = true;

    if (g_style_buf && g_lang != LangId::None) {
        if (del > 0) g_style_buf->remove(pos, pos + del);
        if (ins > 0) {
            std::string pad(ins, ST_NORMAL);
            g_style_buf->insert(pos, pad.c_str());
        }
    }

    schedule_syntax_highlight();
    update_gutter_width();
}

// ============================================================
// main
// ============================================================
int main(int argc, char** argv) {
    Fl::set_font(FL_FREE_FONT, "Consolas");
    init_languages();

    prefs = new Fl_Preferences(Fl_Preferences::USER, "TeditCo", "Tedit");

    win = new TeditWindow(INIT_W, INIT_H, "TEDIT");
    win->begin();

    win->menubar = new Fl_Menu_Bar(0, 0, INIT_W, MENU_H);
    win->menubar->add("&Файл/&Новый",             FL_CTRL + 'n',            menu_new);
    win->menubar->add("&Файл/&Открыть...",        FL_CTRL + 'o',            menu_open);
    win->menubar->add("&Файл/&Сохранить",         FL_CTRL + 's',            menu_save);
    win->menubar->add("&Файл/Со&хранить как...",  FL_CTRL + FL_SHIFT + 's', menu_save_as);
    win->menubar->add("&Файл/sep");
    win->menubar->add("&Файл/sep");
    win->menubar->add("&Файл/Снэпшоты/&Создать (Ctrl+Alt+S)", FL_CTRL + FL_ALT + 's', menu_snapshot_create);
    win->menubar->add("&Файл/Снэпшоты/&История...",           0,                        menu_snapshot_history);
    win->menubar->add("&Файл/Снэпшоты/Открыть &папку",        0,                        menu_snapshot_folder);
    win->menubar->add("&Файл/sep");
    win->menubar->add("&Файл/В&ыход",             FL_CTRL + 'q',            menu_quit);

    win->menubar->add("&Правка/&Отменить",              FL_CTRL + 'z',              menu_undo);
    win->menubar->add("&Правка/&Повторить",             FL_CTRL + 'y',              menu_redo);
    win->menubar->add("&Правка/sep");
    win->menubar->add("&Правка/В&ырезать",              FL_CTRL + 'x',              menu_cut);
    win->menubar->add("&Правка/&Копировать",            FL_CTRL + 'c',              menu_copy);
    win->menubar->add("&Правка/Копировать как &ASCII",  FL_CTRL + FL_SHIFT + 'c',   menu_copy_ascii);
    win->menubar->add("&Правка/&Вставить",              FL_CTRL + 'v',              menu_paste);
    win->menubar->add("&Правка/sep");
    win->menubar->add("&Правка/В&ыделить всё",          FL_CTRL + 'a',              menu_select_all);

    win->menubar->add("&Поиск/&Найти...",         FL_CTRL + 'f',            menu_find);
    win->menubar->add("&Поиск/&Заменить...",      FL_CTRL + 'h',            menu_find_repl);
    win->menubar->add("&Поиск/sep");
    win->menubar->add("&Поиск/Найти &далее",      FL_F + 3,                 menu_find_next);
    win->menubar->add("&Поиск/Найти &ранее",      FL_SHIFT + FL_F + 3,      menu_find_prev);
    win->menubar->add("&Поиск/sep");
    win->menubar->add("&Поиск/&Перейти...",       FL_CTRL + 'g',            menu_goto);

    win->menubar->add("&Вид/&Режим: Текст",       0,           menu_mode_text);
    win->menubar->add("&Вид/&Режим: HEX",         0,           menu_mode_hex);
    win->menubar->add("&Вид/&Переключить (F4)",   FL_F + 4,    menu_mode_toggle);
    win->menubar->add("&Вид/sep");
    win->menubar->add("&Вид/&Увеличить шрифт",    FL_CTRL + '=', menu_zoom_in);
    win->menubar->add("&Вид/&Уменьшить шрифт",    FL_CTRL + '-', menu_zoom_out);
    win->menubar->add("&Вид/&Сбросить шрифт",     FL_CTRL + '0', menu_zoom_reset);
    win->menubar->add("&Вид/sep");
    win->menubar->add("&Вид/&Номера строк",        0, menu_toggle_linenumbers, 0, FL_MENU_TOGGLE);
    win->menubar->add("&Вид/sep");
    win->menubar->add("&Вид/&Тема/&Тёмная",  0, menu_theme_dark,  0, FL_MENU_RADIO);
    win->menubar->add("&Вид/&Тема/&Светлая", 0, menu_theme_light, 0, FL_MENU_RADIO);
    win->menubar->add("&Вид/&Тема/&Сепия",   0, menu_theme_sepia, 0, FL_MENU_RADIO);
    win->menubar->add("&Вид/&Перенос строк",       0, menu_toggle_wordwrap,    0, FL_MENU_TOGGLE);

    tbuf = new Fl_Text_Buffer();
    win->text_ed = new CalmEditor(0, MENU_H, INIT_W, INIT_H - MENU_H - STATUS_H);
    win->text_ed->buffer(tbuf);
    win->text_ed->textfont(FL_FREE_FONT);
    win->text_ed->textsize(text_size);
    win->text_ed->color(COLOR_BG);
    win->text_ed->textcolor(COLOR_TEXT);
    win->text_ed->cursor_color(COLOR_BG);
    win->text_ed->selection_color(COLOR_SELECTION);
    win->text_ed->linenumber_width(GUTTER_W);
    win->text_ed->linenumber_font(FL_FREE_FONT);
    win->text_ed->linenumber_size(text_size);
    win->text_ed->linenumber_fgcolor(COLOR_GUTTER_FG);
    win->text_ed->linenumber_bgcolor(COLOR_GUTTER);
    win->text_ed->linenumber_align(FL_ALIGN_RIGHT);

    g_style_buf = new Fl_Text_Buffer();
    for (int i = 0; i < G_STYLE_COUNT; ++i) g_style_table[i].size = text_size;
    win->text_ed->highlight_data(g_style_buf, g_style_table, G_STYLE_COUNT, ST_NORMAL, 0, 0);

    win->hex_view = new HexView(0, MENU_H, INIT_W - 16, INIT_H - MENU_H - STATUS_H);
    win->hex_view->callback([](Fl_Widget*, void*) { update_status(); });
    win->hex_vsb  = new Fl_Scrollbar(INIT_W - 16, MENU_H, 16, INIT_H - MENU_H - STATUS_H);
    win->hex_vsb->type(FL_VERTICAL);
    win->hex_vsb->linesize(1);
    win->hex_view->attach_scrollbar(win->hex_vsb);
    win->hex_view->set_doc(&hdoc);

    int y = INIT_H - STATUS_H;
    auto cell = [&](int x, int w, Fl_Align a, const char* init) {
        Fl_Box* b = new Fl_Box(x, y, w, STATUS_H);
        b->box(FL_FLAT_BOX);
        b->color(COLOR_STATUS);
        b->labelcolor(COLOR_STATUS_FG);
        b->align(a | FL_ALIGN_INSIDE);
        b->labelsize(12);
        b->copy_label(init);
        return b;
    };
    win->status_pos  = cell(0,   180, FL_ALIGN_LEFT,  "Ln 1, Col 1");
    win->status_mode = cell(180,  60, FL_ALIGN_LEFT,  "TEXT");
    win->status_info = cell(240, 300, FL_ALIGN_LEFT,  "");
    win->status_file = cell(540, INIT_W-540-140, FL_ALIGN_LEFT, "untitled");
    win->status_size = cell(INIT_W-140, 140, FL_ALIGN_RIGHT, "0 B");
    win->status_info->labelcolor(COLOR_STATUS_ACC);

    win->end();
    win->callback(win_close_cb);

    win->show(argc, argv);
    load_state();

    tbuf->add_modify_callback(on_text_modify, nullptr);
    Fl::add_timeout(0.25, status_tick);
    win->text_ed->start_calm_cursor();

    update_title();
    update_status();

    if (argc > 1) {
        open_file(argv[1]);
    } else {
        // Восстановление прошлой сессии
        char last_buf[1024] = {0};
        if (prefs) prefs->get("last_file", last_buf, "", sizeof(last_buf));
        std::string last = last_buf;
        if (!last.empty()) {
            DWORD attrs = GetFileAttributesA(last.c_str());
            if (attrs != INVALID_FILE_ATTRIBUTES && !(attrs & FILE_ATTRIBUTE_DIRECTORY)) {
                int saved_mode       = 0;
                int saved_cursor     = 0;
                int saved_hex_top    = 0;
                int saved_hex_cursor = 0;
                int saved_ascii      = 0;
                prefs->get("last_mode",       saved_mode,       0);
                prefs->get("last_cursor_pos", saved_cursor,     0);
                prefs->get("last_hex_top",    saved_hex_top,    0);
                prefs->get("last_hex_cursor", saved_hex_cursor, 0);
                prefs->get("last_hex_ascii",  saved_ascii,      0);

                open_file(last, false);

                // Если в сессии был HEX, но файл открылся в TEXT — переключаем
                if (saved_mode == 1 && win->mode == Mode::TEXT
                    && hdoc.is_open() && hdoc.length() <= TEXT_MODE_LIMIT) {
                    win->set_mode(Mode::HEX);
                }

                if (win->mode == Mode::TEXT && saved_cursor > 0) {
                    int pos = std::min(saved_cursor, tbuf->length());
                    win->text_ed->insert_position(pos);
                    win->text_ed->show_insert_position();
                } else if (win->mode == Mode::HEX && hdoc.is_open() && hdoc.length() > 0) {
                    size_t off = (size_t)saved_hex_cursor;
                    if (off >= hdoc.length()) off = hdoc.length() - 1;
                    win->hex_view->cursor_ = off;
                    win->hex_view->in_ascii_ = (saved_ascii != 0);
                    size_t top = (size_t)saved_hex_top;
                    if (top >= hdoc.length()) top = 0;
                    win->hex_view->top_ = top;
                    win->hex_view->redraw();
                }
                update_title();
                update_status();
            }
        }
    }

    int rc = Fl::run();
    save_state();
    if (win && win->text_ed) win->text_ed->stop_calm_cursor();
    Fl::remove_timeout(status_tick);
    Fl::remove_timeout(syntax_debounce_cb);
    delete g_style_buf;
    delete prefs;
    return rc;
}