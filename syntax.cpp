// syntax.cpp — подсветка синтаксиса (реализация)
#include "syntax.h"
#include <cctype>
#include <cstring>
#include <initializer_list>
#include <string_view>
#include <unordered_map>

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

void init_languages() {
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

LangId detect_language(const std::string& path, const char* text, int len) {
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

void tokenize_buffer(const char* text, int len, LangId lang, std::vector<char>& out) {
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