// hex.cpp — HEX-режим (реализация)
#include "hex.h"

#include <algorithm>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <utility>
#include <FL/fl_draw.H>

// ============================================================
// MappedFile
// ============================================================
bool MappedFile::open(const std::string& path) {
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

void MappedFile::close() {
    if (data_) { UnmapViewOfFile(data_); data_ = nullptr; }
    if (hMap)  { CloseHandle(hMap);      hMap  = nullptr; }
    if (hFile != INVALID_HANDLE_VALUE) { CloseHandle(hFile); hFile = INVALID_HANDLE_VALUE; }
    size_ = 0;
}

// ============================================================
// HexDoc
// ============================================================
void HexDoc::set_byte(size_t off, uint8_t val, bool record) {
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

void HexDoc::set_bytes(size_t off, const std::vector<uint8_t>& vals) {
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

bool HexDoc::undo_last() {
    if (undo_stack.empty()) return false;
    HexEditRecord rec = std::move(undo_stack.back());
    undo_stack.pop_back();
    for (size_t i = 0; i < rec.old_vals.size(); ++i)
        set_byte(rec.off + i, rec.old_vals[i], false);
    redo_stack.push_back(std::move(rec));
    return true;
}

bool HexDoc::redo_last() {
    if (redo_stack.empty()) return false;
    HexEditRecord rec = std::move(redo_stack.back());
    redo_stack.pop_back();
    for (size_t i = 0; i < rec.new_vals.size(); ++i)
        set_byte(rec.off + i, rec.new_vals[i], false);
    undo_stack.push_back(std::move(rec));
    return true;
}

void HexDoc::reset() {
    file.close();
    edits.clear();
    undo_stack.clear();
    redo_stack.clear();
}

bool HexDoc::save(const std::string& path) {
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

// ============================================================
// HexView — базовое
// ============================================================
void HexView::set_doc(HexDoc* d) {
    doc_ = d; top_ = cursor_ = 0; pending_ = -1;
    sel_start_ = sel_end_ = sel_anchor_ = 0;
    sel_active_ = false;
    matches.clear(); current_match = -1;
    sync_scrollbar(); redraw();
}

void HexView::attach_scrollbar(Fl_Scrollbar* s) {
    vsb_ = s; vsb_->callback(scrollbar_cb, this); sync_scrollbar();
}

void HexView::clear_matches() {
    matches.clear(); current_match = -1; redraw();
}

void HexView::set_matches(const std::vector<HexMatch>& m) {
    matches = m; current_match = -1; redraw();
}

void HexView::clear_selection() {
    sel_start_ = sel_end_ = 0; sel_active_ = false; redraw();
}

void HexView::copy_selection(bool as_hex) {
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

void HexView::paste_hex() {
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
        tedit_show_alert("Вставка", "В буфере нет корректной hex-последовательности.");
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

void HexView::select_all() {
    if (!doc_ || doc_->length() == 0) return;
    sel_start_ = 0;
    sel_end_ = doc_->length();
    sel_anchor_ = 0;
    cursor_ = sel_end_ - 1;
    sel_active_ = true;
    ensure_visible();
    redraw();
}

void HexView::goto_offset(size_t off, bool set_cursor) {
    if (!doc_ || !doc_->is_open()) return;
    if (off >= doc_->length()) off = doc_->length() > 0 ? doc_->length() - 1 : 0;
    if (set_cursor) cursor_ = off;
    top_ = (off / BPR) * BPR;
    sync_scrollbar();
    redraw();
}

void HexView::find_next() {
    if (matches.empty()) return;
    current_match = (current_match + 1) % (int)matches.size();
    goto_offset(matches[current_match].offset);
}

void HexView::find_prev() {
    if (matches.empty()) return;
    current_match = current_match - 1;
    if (current_match < 0) current_match = (int)matches.size() - 1;
    goto_offset(matches[current_match].offset);
}

void HexView::show_context_menu() {
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

// ============================================================
// HexView — handle
// ============================================================
int HexView::handle(int e) {
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

void HexView::resize(int X, int Y, int W, int H) {
    Fl_Widget::resize(X, Y, W, H); sync_scrollbar();
}

// ============================================================
// HexView — draw
// ============================================================
void HexView::draw() {
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

// ============================================================
// HexView — private
// ============================================================
void HexView::update_selection() {
    if (!sel_active_) { sel_start_ = sel_end_ = 0; return; }
    size_t a = std::min(sel_anchor_, cursor_);
    size_t b = std::max(sel_anchor_, cursor_);
    sel_start_ = a;
    sel_end_   = b + 1;
}

void HexView::scroll_to(size_t off) {
    top_ = off; sync_scrollbar(); redraw();
}

void HexView::ensure_visible() {
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

int HexView::handle_key() {
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

bool HexView::byte_at_mouse(int mx, int my, size_t* out_off, bool* out_ascii) {
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

void HexView::scrollbar_cb(Fl_Widget* w, void* d) {
    HexView* s = static_cast<HexView*>(d);
    s->top_ = (size_t)((Fl_Scrollbar*)w)->value();
    s->redraw();
}

void HexView::sync_scrollbar() {
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

// ============================================================
// Поиск в HEX (чистые функции, без UI)
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
    while (off + nlen <= len && result.size() < 100000) {
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
                if (result.size() >= 100000) break;
            }
        }
        std::sort(result.begin(), result.end(),
                  [](const HexMatch& a, const HexMatch& b){ return a.offset < b.offset; });
    }
    return result;
}