package spanlang

import "core:testing"

@(test)
parser_empty :: proc(t: ^testing.T) {
    source := ""
    tokens: [dynamic]Token
    defer delete(tokens)
    lex_content(source, &tokens)

    stmts: [dynamic]Stmt
    defer delete(stmts)
    parse_program(tokens[:], &stmts)

    testing.expectf(t, 0 == len(stmts),
        "length: expected=0, got=%d", len(stmts))
}

@(test)
parser_negate_op :: proc(t: ^testing.T) {
	source := "-2; --3; 4; -5;"
	expecteds := []Stmt {
		Expr_Stmt{make_lit(f32(-2))},
		Expr_Stmt{make_lit(f32( 3))},
		Expr_Stmt{make_lit(f32( 4))},
		Expr_Stmt{make_lit(f32(-5))},
	}

	parse_tester(t, source, expecteds)
	free_all(context.allocator)
}

@(test)
parser_binary_ops :: proc(t: ^testing.T) {
	source := "2 + 3; 5 * 4 + 2;"
	expecteds := []Stmt {
		Expr_Stmt{
			make_binary(
				make_lit(f32(2)),
				.Add,
				make_lit(f32(3)),
			)
		},
		Expr_Stmt{
			make_binary(
				make_binary(
					make_lit(f32(5)),
					.Multiply,
					make_lit(f32(4)),
				),
				.Add,
				make_lit(f32(2)),
			)
		},
	}

	parse_tester(t, source, expecteds)
	free_all(context.allocator)
}

@(private="file")
make_lit :: proc(literal: any, alloc := context.temp_allocator) -> ^Expr {
	switch lit in literal {
	case f32: return new_clone(Expr(Literal(lit)), alloc)
	case string: return new_clone(Expr(Literal(lit)), alloc)
	case Vector: return new_clone(Expr(Literal(lit)), alloc)
	case: unreachable()
	}
}

@(private="file")
make_binary :: proc(
	left: ^Expr, op: Operator, right: ^Expr, alloc := context.temp_allocator
) -> ^Expr {
	return new_clone(Expr(Infix_Expr{left=left, op=op, right=right}), alloc)
}

@(private="file")
parse_tester :: proc(
	t: ^testing.T, source: string, expecteds: []Stmt, loc := #caller_location
) {
	tokens: [dynamic]Token
    defer delete(tokens)
    lex_content(source, &tokens)

    stmts: [dynamic]Stmt
    defer delete(stmts)
    parse_program(tokens[:], &stmts)

    testing.expectf(t, len(expecteds) == len(stmts),
        "length: expected=%d, got=%d", len(expecteds), len(stmts), loc=loc)

    for i in 0..<len(expecteds) {
        testing.expectf(t, is_stmt_eq(expecteds[i], stmts[i]),
        	"value: expected=%v, got=%v", expecteds[i], stmts[i], loc=loc)
    }
}

@(private="file")
is_stmt_eq :: proc(a, b: Stmt) -> bool {
	switch st_a in a {
	case Call_Stmt:
		cs_b, ok := b.(Call_Stmt)
		if ok do return st_a == cs_b
		else do return false
	case Def_Stmt:
		ds_b, ok := b.(Def_Stmt)
		if ok do return st_a == ds_b
		else do return false
	case Expr_Stmt:
		es_b, ok := b.(Expr_Stmt)
		if ok do return is_expr_eq(st_a.expr^, es_b.expr^)
		else do return false
	}
	return false
}
