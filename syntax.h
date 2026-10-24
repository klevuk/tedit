// syntax.h — подсветка синтаксиса (интерфейс)
#pragma once

#include <string>
#include <vector>

// Стили (каждый — индекс в таблице, буква ASCII):
// A normal, B keyword, C type, D string, E comment, F number,
// G preprocessor, H operator, I escape, J attribute/marker, K tag
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

// Инициализация таблиц языков (вызывается один раз при старте).
void init_languages();

// Определение языка по имени файла и содержимому.
LangId detect_language(const std::string& path, const char* text, int len);

// Заполнить out стилями для текста [text, text+len). out.size() == len.
void tokenize_buffer(const char* text, int len, LangId lang, std::vector<char>& out);