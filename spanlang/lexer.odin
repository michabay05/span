package spanlang

import "core:fmt"
import "core:strings"

Token_Kind :: enum {
    // Keywords
    Scene,
    If,

    // Keywords - units
    ViewWidth,
    ViewHeight,
    Degrees,

    // "Variable" tokens
    Identifier,
    Number,
    Text,

    // Symbols
    Assign,
    Equals,
    Plus,
    Minus,
    Asterisk,
    Slash,

    // Punctuation
    OCurly,
    CCurly,
    OParen,
    CParen,
    Colon,
    OBracket,
    CBracket,
    Comma,
    Semicolon,
}

Token :: struct {
    kind: Token_Kind,
    literal: string,
}

lex_content :: proc(content: string, tokens: ^[dynamic]Token) {
    current := 0
    line := 1

    for current < len(content) {
        ch, avail := peek(content, current)
        if !avail do break;

        switch ch {
        case '=':
            second_ch, avail := peek(content, current, 1)
            token := Token{kind = .Assign, literal = consume(content, &current)}
            if (avail && second_ch == '=') {
                token.kind = .Equals
                token.literal = strings.concatenate(
                    []string{token.literal, consume(content, &current)})
            }
            append(tokens, token)
        case '(':
            append(tokens, Token{kind = .OParen, literal = consume(content, &current)})
        case ')':
            append(tokens, Token{kind = .CParen, literal = consume(content, &current)})
        case '{':
            append(tokens, Token{kind = .OCurly, literal = consume(content, &current)})
        case '}':
            append(tokens, Token{kind = .CCurly, literal = consume(content, &current)})
        case '[':
            append(tokens, Token{kind = .OBracket, literal = consume(content, &current)})
        case ']':
            append(tokens, Token{kind = .CBracket, literal = consume(content, &current)})
        case ':':
            append(tokens, Token{kind = .Colon, literal = consume(content, &current)})
        case ';':
            append(tokens, Token{kind = .Semicolon, literal = consume(content, &current)})
        case '+':
            append(tokens, Token{kind = .Plus, literal = consume(content, &current)})
        case '-':
            append(tokens, Token{kind = .Minus, literal = consume(content, &current)})
        case '*':
            append(tokens, Token{kind = .Asterisk, literal = consume(content, &current)})
        case '/':
        	// TODO: add single-line comments here
            append(tokens, Token{kind = .Slash, literal = consume(content, &current)})
        case ',':
            append(tokens, Token{kind = .Comma, literal = consume(content, &current)})
        case '\n', '\r':
            line += 1
            fallthrough
        case ' ', '\t': current += 1
        case:
            if is_alpha(ch) {
                ident := consume_identifier(content, &current)
                append(tokens, classify_identifer(ident))
            } else if is_digit(ch) {
                number := consume_number(content, &current)
                append(tokens, Token{kind = .Number, literal = number})
            } else if ch == '"' {
                // Skip the starting quote
                current += 1
                text := consume_text(content, &current)
                // Skip the ending quote
                current += 1
                append(tokens, Token{kind = .Text, literal = text})
            } else {
                fmt.printfln("Unknown character: '%c' at line %d", ch, line)
                unreachable()
            }
        }
    }
}

classify_identifer :: proc(ident: string) -> Token {
    token := Token{kind = .Identifier, literal = ident}
    switch ident {
    case "scene" : token.kind = .Scene
    case "if" : token.kind = .If
    case "vw" : token.kind = .ViewWidth
    case "vh" : token.kind = .ViewHeight
    case "deg": token.kind = .Degrees
    }
    return token
}

peek :: proc(content: string, current: int, ahead: int = 0) -> (u8, bool) {
    if is_at_end(content, current + ahead) do return 0, false;
    return content[current + ahead], true
}

consume :: proc(content: string, current: ^int) -> string {
    if is_at_end(content, current^) do unreachable()
    ch := content[current^:current^ + 1]
    current^ += 1
    return ch
}

consume_identifier :: proc(content: string, current: ^int) -> string {
    start := current^
    for !is_at_end(content, current^) && (is_aldigit(content[current^]) || content[current^] == '_') {
        current^ += 1
    }
    return content[start:current^]
}

consume_number :: proc(content: string, current: ^int) -> string {
    start := current^
    for !is_at_end(content, current^) && is_numeric(content[current^]) do current^ += 1
    return content[start:current^]
}

consume_text :: proc(content: string, current: ^int) -> string {
    start := current^
    for !is_at_end(content, current^) && content[current^] != '"' do current^ += 1
    return content[start:current^]
}

is_alpha :: #force_inline proc(ch: u8) -> bool {
    return (u8('A') <= ch && ch <= u8('Z')) || (u8('a') <= ch && ch <= u8('z'))
}

is_digit :: #force_inline proc(ch: u8) -> bool {
    return u8('0') <= ch && ch <= u8('9')
}

is_numeric :: #force_inline proc(ch: u8) -> bool {
    return is_digit(ch) || ch == '.'
}

is_aldigit :: #force_inline proc(ch: u8) -> bool {
    return is_alpha(ch) || is_digit(ch)
}

@(private="file")
is_at_end :: #force_inline proc(content: string, current: int) -> bool {
    return current >= len(content)
}
