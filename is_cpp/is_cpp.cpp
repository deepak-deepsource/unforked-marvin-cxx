#include <fstream>
#include <iostream>
#include <optional>
#include <unordered_set>
#include <cstring>

// C++20 only code!

struct Span {
    size_t start, end;
};

struct Token {
    bool non_source;
    const char* non_source_end;
    size_t non_source_end_len;
    bool not_ident;
    const char* src;
    size_t src_len;
    Span span;
};

const size_t MAX_TOKEN_SIZE = 102400;
struct ParseState {
    char tok[MAX_TOKEN_SIZE];
    size_t toki;
    size_t last_parsed;
    bool non_source;
    Token last_token;
};

std::optional<Token> try_parse(ParseState& ps, bool hm) {
    bool not_ident = false;
    bool non_source = ps.non_source;
    char* non_source_end = "";
    size_t non_source_end_len = 0;
    size_t tok_start = 0;
    size_t tok_end = 0;
    for (size_t i = 0; i < ps.toki; i++) {
        const char ch = ps.tok[i];
        // heuristic tokenizing logic
        if (non_source) {
            // find 'source_end_delim'
            if (!isalnum(ch)) {
                if (ps.last_token.non_source_end_len > 1) {
                    auto ch2 = 0;
                    if (i < ps.toki - 1) {
                        ch2 = ps.tok[i + 1];
                    }
                    if (ch == ps.last_token.non_source_end[0] &&
                        ch2 == ps.last_token.non_source_end[1]) {
                        non_source = false;
                        break;
                    } else {
                        tok_end++;
                    }
                } else {
                    if (ch == ps.last_token.non_source_end[0]) {
                        non_source = false;
                        break;
                    } else {
                        tok_end++;
                    }
                }
            } else {
                tok_end++;
            }
            continue;
        }

        if (!isalnum(ch)) {
            if (tok_start != tok_end) {
                tok_end--;
                // std::cout << "str?: " << ps.tok << std::endl;
                break;
            }
            auto string = ch == '"';
            if (string) {
                non_source = true;
                non_source_end = "\"";
                non_source_end_len = 1;
            }
            auto ch2 = 0;
            if (i < ps.toki - 1) {
                ch2 = ps.tok[i + 1];
            } else if (hm) {
                return {};
            }
            auto single_line_comment = ch == '/' && ch2 == '/';
            auto multi_line_comment = ch == '/' && ch2 == '*';
            if (single_line_comment) {
                tok_end++;
                non_source = true;
                non_source_end = "\n";
                non_source_end_len = 1;
            }
            if (multi_line_comment) {
                tok_end++;
                non_source = true;
                non_source_end = "*/";
                non_source_end_len = 2;
            }
            auto namespacer = ch == ':' && ch2 == ':';
            auto streamout = ch == '<' && ch2 == '<';
            auto streamin = ch == '>' && ch2 == '>';
            if (namespacer || streamin || streamout) {
                tok_end++;
            }
            not_ident = true;
            break;
        } else {
            tok_end++;
        }
    }
    if (non_source && hm) {
        return {};
    }
    auto t = Token{
        .non_source = non_source,
        .non_source_end = non_source_end,
        .non_source_end_len = non_source_end_len,
        .not_ident = not_ident,
        .src = ps.tok + tok_start,
        .src_len = tok_end - tok_start + 1,
        .span = {.start = ps.last_parsed + tok_start, .end = ps.last_parsed + tok_end},
    };
    return t;
}

bool in_cpp_keyword_set(Token t) {
    const std::unordered_set<std::string> keyword_set = {
        "using",     "namespace", "auto",     "::",       ">>",        "<<",     "template",
        "typename",  "class",     "public",   "private",  "protected", "friend", "constexpr",
        "consteval", "coroutine", "noexcept", "explicit", "operator",  "new",    "mutable"};
    return keyword_set.count(std::string(t.src, t.src_len)) == 1;
}

bool parser(std::filebuf* fb, ParseState& ps, bool has_more = true) {
    auto res = try_parse(ps, has_more);
    if (res.has_value()) {
        auto token = res.value();
        ps.last_token = token;
#ifdef DEBUG
        std::cout << "Token src: " << std::string(token.src, token.src_len) << std::endl;
        std::cout << "Token src_len: " << token.src_len << std::endl;
        std::cout << "Non-Source?: " << token.non_source << std::endl;
        std::cout << "----------------------------------" << std::endl;
#endif
        if (!token.non_source && !ps.non_source) {
            if (in_cpp_keyword_set(token)) {
               return true;
            }
        }
        // skip non source
        if (token.non_source) {
            ps.non_source = true;
        } else {
            ps.non_source = false;
        }
        // ensure tok is reset
        auto end = token.span.end + 1;
        auto copied = end - ps.last_parsed;
        if (ps.toki > copied) {
            auto rest = ps.toki - copied;
            std::memcpy(ps.tok, ps.tok + copied, rest);
            std::memset(ps.tok + rest, 0, copied);
            ps.toki = rest;
        } else {
            std::memset(ps.tok, 0, ps.toki);
            ps.toki = 0;
        }
        ps.last_parsed = end;
    }
    return false;
}

// TODO: OpenMP for parallelism
int main(int argc, char** argv) {
    if (argc <= 1) {
        std::cerr << "Pls provide a path to file to check." << std::endl;
        return 0;
    }
    int retval = -1;
    // File I/O
    std::ifstream is;
    std::filebuf* fb = is.rdbuf();
    fb->open(argv[1], std::ios::in);
    // buffered parsing
    ParseState ps = {};
    do {
        char ch = fb->sgetc();
        ps.tok[ps.toki++] = ch;
        if (!isalnum(ch)) {
            if (parser(fb, ps)) {
                retval = 0;
                goto cleanup;
            }
        }
    } while (fb->snextc() != EOF);
    while (*ps.tok != '\0') { // we std::memset tok to '\0' after reading
        if (parser(fb, ps, false)) {
            retval = 0;
            goto cleanup;
        }
    }
cleanup:
    fb->close();
    return retval;
}