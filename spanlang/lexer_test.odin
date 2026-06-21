package spanlang

import "core:fmt"
import "core:testing"

@(test)
lexer_empty :: proc(t: ^testing.T) {
    source := ""
    tokens: [dynamic]Token
    defer delete(tokens)
    lex_content(source, &tokens)

    testing.expectf(t, 0 == len(tokens),
        "length: expected=0, got=%d", len(tokens))
}

@(test)
lexer_source_w_whitespace :: proc(t: ^testing.T) {
    source := "    r1  :   = \n   rect(  \n\t )   ;  "
    expecteds := []Token {
        {kind = .Identifier, literal = "r1"},
        {kind = .Colon     , literal = ":"},
        {kind = .Assign    , literal = "="},
        {kind = .Identifier, literal = "rect"},
        {kind = .OParen    , literal = "("},
        {kind = .CParen    , literal = ")"},
        {kind = .Semicolon , literal = ";"},
    }

    tokens: [dynamic]Token
    defer delete(tokens)
    lex_content(source, &tokens)
    testing.expectf(t, len(expecteds) == len(tokens),
        "length: expected=%d, got=%d", len(expecteds), len(tokens))

    for i in 0..<len(expecteds) {
        testing.expectf(t, expecteds[i] == tokens[i],
            "value: expected=%v, got=%v", expecteds[i], tokens[i])
    }
}

@(test)
lexer_units :: proc(t: ^testing.T) {
    source := "rotate(r1, 90deg); translate(r2, [0.314vw, 0.272vh]);"
    expecteds := []Token {
        {kind = .Identifier, literal = "rotate"},
        {kind = .OParen    , literal = "("},
        {kind = .Identifier, literal = "r1"},
        {kind = .Comma     , literal = ","},
        {kind = .Number    , literal = "90"},
        {kind = .Degrees   , literal = "deg"},
        {kind = .CParen    , literal = ")"},
        {kind = .Semicolon , literal = ";"},

        {kind = .Identifier, literal = "translate"},
        {kind = .OParen    , literal = "("},
        {kind = .Identifier, literal = "r2"},
        {kind = .Comma     , literal = ","},
        {kind = .OBracket  , literal = "["},
        {kind = .Number    , literal = "0.314"},
        {kind = .ViewWidth , literal = "vw"},
        {kind = .Comma     , literal = ","},
        {kind = .Number    , literal = "0.272"},
        {kind = .ViewHeight, literal = "vh"},
        {kind = .CBracket  , literal = "]"},
        {kind = .CParen    , literal = ")"},
        {kind = .Semicolon , literal = ";"},
    }

    tokens: [dynamic]Token
    defer delete(tokens)
    lex_content(source, &tokens)
    testing.expectf(t, len(expecteds) == len(tokens),
        "length: expected=%d, got=%d", len(expecteds), len(tokens))

    for i in 0..<len(expecteds) {
        testing.expectf(t, expecteds[i] == tokens[i],
            "value: expected=%v, got=%v", expecteds[i], tokens[i])
    }
}

@(test)
lexer_punctuation :: proc(t: ^testing.T) {
    source := "(){}[],;"
    expecteds := []Token {
        {kind = .OParen   , literal = "("},
        {kind = .CParen   , literal = ")"},
        {kind = .OCurly   , literal = "{"},
        {kind = .CCurly   , literal = "}"},
        {kind = .OBracket , literal = "["},
        {kind = .CBracket , literal = "]"},
        {kind = .Comma    , literal = ","},
        {kind = .Semicolon, literal = ";"},
    }

    tokens: [dynamic]Token
    defer delete(tokens)
    lex_content(source, &tokens)
    testing.expectf(t, len(expecteds) == len(tokens),
        "length: expected=%d, got=%d", len(expecteds), len(tokens))

    for i in 0..<len(expecteds) {
        testing.expectf(t, expecteds[i] == tokens[i],
            "value: expected=%v, got=%v", expecteds[i], tokens[i])
    }
}

@(test)
lexer_keywords :: proc(t: ^testing.T) {
    source := "scene scenes if"
    expecteds := []Token {
        {kind = .Scene     , literal = "scene"},
        {kind = .Identifier, literal = "scenes"},
        {kind = .If        , literal = "if"},
    }

    tokens: [dynamic]Token
    defer delete(tokens)
    lex_content(source, &tokens)
    testing.expectf(t, len(expecteds) == len(tokens),
        "length: expected=%d, got=%d", len(expecteds), len(tokens))

    for i in 0..<len(expecteds) {
        testing.expectf(t, expecteds[i] == tokens[i],
            "value: expected=%v, got=%v", expecteds[i], tokens[i])
    }
}

@(test)
lexer_text :: proc(t: ^testing.T) {
    source := "t1 := text(\"How are you doing, **today**?\")"
    expecteds := []Token {
        {kind = .Identifier, literal = "t1"},
        {kind = .Colon     , literal = ":"},
        {kind = .Assign    , literal = "="},
        {kind = .Identifier, literal = "text"},
        {kind = .OParen    , literal = "("},
        {kind = .Text      , literal = "How are you doing, **today**?"},
        {kind = .CParen    , literal = ")"},
    }

    tokens: [dynamic]Token
    defer delete(tokens)
    lex_content(source, &tokens)
    testing.expectf(t, len(expecteds) == len(tokens),
        "length: expected=%d, got=%d", len(expecteds), len(tokens))

    for i in 0..<len(expecteds) {
        testing.expectf(t, expecteds[i] == tokens[i],
            "value: expected=%v, got=%v", expecteds[i], tokens[i])
    }
}
