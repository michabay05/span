package spanlang

import "core:fmt"
import "core:strconv"

Def_Stmt :: struct {
	ident: Identifier,
	type: string,
	expr: ^Expr,
}

Call_Stmt :: struct {
	name: string,
}

Expr_Stmt :: struct {
	expr: ^Expr,
}

Stmt :: union {
	Def_Stmt,
	Call_Stmt,
	Expr_Stmt,
}

Operator :: enum {
	Add,
	Sub,
	Multiply,
	Divide,
	Negate,
}
Expr :: union {
	Identifier,
	Literal,
	Unary_Expr,
	Binary_Expr,
}
Unary_Expr :: struct {
	op:    Operator,
	right: ^Expr,
}
Binary_Expr :: struct {
	op:    Operator,
	left:  ^Expr,
	right: ^Expr,
}

Literal :: union {
	f32,
	string,
}
Identifier :: struct {
	name: string,
}

PrefixParseFns :: map[Token_Kind]proc(p: ^Parser, alloc := context.allocator) -> ^Expr
InfixParseFns :: map[Token_Kind]proc(p: ^Parser, left: ^Expr, alloc := context.allocator) -> ^Expr
Parser :: struct {
	tokens:     []Token,
	curr:       int,
	stmts:      ^[dynamic]Stmt,
	prefix_fns: PrefixParseFns,
	infix_fns:  InfixParseFns,
}

Precedence :: enum {
	Lowest,
	Additive,
	Multiplicative,
	Prefix,
	Grouped,
}

parse_program :: proc(tokens: []Token, stmts: ^[dynamic]Stmt) -> Parser {
	p := Parser {
		tokens     = tokens,
		curr       = 0,
		stmts      = stmts,
		prefix_fns = make(PrefixParseFns),
		infix_fns  = make(InfixParseFns),
	}
	_add_prefix_fns(&p)
	_add_infix_fns(&p)

	for p.curr < len(p.tokens) {
		token, ok := _peek_token(&p)
		assert(ok)
		#partial switch token.kind {
		case .Identifier:
			append(stmts, _parse_ident_stmt(&p))
		case:
			append(stmts, Stmt(_parse_expr_stmt(&p)))
		}
	}

	return p
}

dump_stmt :: proc(stmt: Stmt) {
	#partial switch st in stmt {
	case Expr_Stmt:
		_dump_expr(st.expr)
		fmt.println(";")
	case Def_Stmt:
		fmt.printf("%s", st.ident.name)
		if len(st.type) > 0 {
			fmt.print(": %s ", st.type)
		} else {
			fmt.print(" := ")
		}
		_dump_expr(st.expr)
		fmt.println()
	case Call_Stmt:
		fmt.printfln("%s(<EXPR_TO_BE_DONE>);", st.name)
	case:
		fmt.eprintfln("unhandled kind: %v", st)
		unimplemented()
	}
}

_dump_expr :: proc(expr: ^Expr) {
	if expr == nil do return
	switch ex in expr {
	case Unary_Expr:
		#partial switch ex.op {
		case .Negate:
			fmt.printf("-(")
			_dump_expr(ex.right)
			fmt.printf(")")
		case: unreachable()
		}

	case Binary_Expr:
		fmt.print("(")
		_dump_expr(ex.left)

		switch ex.op {
		case .Add: fmt.print(" + ")
		case .Sub: fmt.print(" - ")
		case .Multiply: fmt.print(" * ")
		case .Divide: fmt.print(" / ")
		case .Negate: unreachable()
		}

		_dump_expr(ex.right)
		fmt.print(")")
	case Literal:
		fmt.print(ex)
	case Identifier:
		fmt.print(ex.name)
	}
}

_parse_ident_stmt :: proc(p: ^Parser) -> Stmt {
	// Peek ahead 0 = ident
	// Peek ahead 1 = (token)
	token, ok := _peek_token(p, ahead=1)
	assert(ok)

	#partial switch token.kind {
	case .Colon:
		ident := Identifier{name = _consume_token(p).literal}
		_, colon_ok := _expect_consume(p, .Colon)
		assert(colon_ok)
		return Stmt(_parse_def_stmt(p, ident))
	case:
		fmt.println("[WARN] stmt starts with token but treated as expr stmt")
		return Stmt(_parse_expr_stmt(p))
	}
}

_parse_expr_stmt :: proc(p: ^Parser) -> Expr_Stmt {
	expr := _parse_expression(p, .Lowest)
	_, sc_ok := _expect_consume(p, .Semicolon)
	assert(sc_ok)
	return Expr_Stmt{expr = expr}
}

_parse_def_stmt :: proc(p: ^Parser, ident: Identifier) -> Def_Stmt {
	def_stmt := Def_Stmt{
		ident = ident,
		type = "",
		expr = nil,
	}

	token, ok := _peek_token(p)
	assert(ok)

	#partial switch token.kind {
	case .Assign:
		assign := _consume_token(p)
		def_stmt.expr = _parse_expression(p, .Lowest)
	case:
		fmt.printfln("[TODO] parse this token for a def stmt: %v", token)
		unimplemented()
	}

	sc, sc_ok := _expect_consume(p, .Semicolon)
	assert(sc_ok)
	return def_stmt
}

_parse_expression :: proc(p: ^Parser, min_prec: Precedence) -> ^Expr {
	first_token, peek_ok := _peek_token(p)
	assert(peek_ok)

	prefix_fn, prefix_ok := p.prefix_fns[first_token.kind]
	if !prefix_ok {
		if first_token.kind != .Semicolon {
			fmt.eprintfln("ERROR: no prefix function found for %v", first_token)
		}
		return nil
	}
	left := prefix_fn(p)

	for {
		token, ok := _peek_token(p)
		if !(ok && token.kind != .Semicolon && min_prec < _get_precedence(token.kind)) {
			break
		}

		infix_fn, infix_ok := p.infix_fns[token.kind]
		if !infix_ok {
			fmt.eprintfln("ERROR: no infix function found for %v", token)
			unreachable()
		}

		left = infix_fn(p, left)
	}

	return left
}

_get_precedence :: proc(kind: Token_Kind) -> Precedence {
	#partial switch kind {
	case .Plus, .Minus:
		return .Additive
	case .Asterisk, .Slash:
		return .Multiplicative
	case:
		return .Lowest
	}
}

_peek_token :: #force_inline proc(p: ^Parser, ahead: int = 0) -> (Token, bool) {
	if _is_at_end(p.tokens, p.curr + ahead) do return {}, false
	return p.tokens[p.curr + ahead], true
}

_consume_token :: proc(p: ^Parser) -> Token {
	token := p.tokens[p.curr]
	p.curr += 1
	return token
}

_expect_consume :: proc(p: ^Parser, kind: Token_Kind) -> (Token, bool) {
	token, ok := _peek_token(p)
	if ok && token.kind == kind {
		return _consume_token(p), true
	} else {
		// TODO: log error here
		return {}, false
	}
}

_add_prefix_fns :: proc(p: ^Parser) {
	p.prefix_fns[.Identifier] = proc(p: ^Parser, alloc := context.allocator) -> ^Expr {
		return new_clone(Expr(Identifier{name = _consume_token(p).literal}))
	}

	p.prefix_fns[.Number] = proc(p: ^Parser, alloc := context.allocator) -> ^Expr {
		num_token := _consume_token(p)
		val, ok := strconv.parse_f32(num_token.literal)
		assert(
			ok,
			fmt.tprintf("Unable to parse '%v' as f32", num_token.literal),
			loc = #location(),
		)
		return new_clone(Expr(Literal(val)), allocator=alloc)
	}

	p.prefix_fns[.OParen] = proc(p: ^Parser, alloc := context.allocator) -> ^Expr {
		oparen := _consume_token(p)
		expr := _parse_expression(p, .Lowest)
		_, ok := _expect_consume(p, .CParen)
		assert(ok, fmt.tprintf("Unable to find matching close paren"), loc = #location())
		return expr
	}

	p.prefix_fns[.Minus] = proc(p: ^Parser, alloc := context.allocator) -> ^Expr {
		negate := _consume_token(p)
		return new_clone(Expr(Unary_Expr{
			op = .Negate,
			right = _parse_expression(p, .Prefix)
		}), allocator=alloc)
	}

	p.prefix_fns[.Text] = proc(p: ^Parser, alloc := context.allocator) -> ^Expr {
		text := _consume_token(p)
		return new_clone(Expr(Literal(text.literal)), allocator=alloc)
	}
}

_add_infix_fns :: proc(p: ^Parser) {
	_parse_infix_expr :: proc(p: ^Parser, left: ^Expr, alloc := context.allocator) -> ^Expr {
		op_token := _consume_token(p)
		op, op_ok := _to_operator(op_token)
		if !op_ok {
			fmt.eprintfln("Could not find op mapping for %v", op_token)
			unreachable()
		}

		prec := _get_precedence(op_token.kind)
		right := _parse_expression(p, prec)

		return new_clone(Expr(Binary_Expr{
			left = left,
			op = op,
			right = right
		}), allocator=alloc)
	}

	p.infix_fns[.Plus] = _parse_infix_expr
	p.infix_fns[.Minus] = _parse_infix_expr
	p.infix_fns[.Asterisk] = _parse_infix_expr
	p.infix_fns[.Slash] = _parse_infix_expr
}

_to_operator :: proc(token: Token) -> (Operator, bool) {
	#partial switch token.kind {
	case .Plus:
		return .Add, true
	case .Minus:
		return .Sub, true
	case .Asterisk:
		return .Multiply, true
	case .Slash:
		return .Divide, true
	case:
		return nil, false
	}
}


@(private = "file")
_is_at_end :: #force_inline proc(tokens: []Token, curr: int) -> bool {
	return curr >= len(tokens)
}

is_expr_eq :: proc(a, b: Expr) -> bool {
	switch ex_a in a {
	case Unary_Expr:
		unimplemented()
	case Binary_Expr:
		be_b, ok := b.(Binary_Expr)
		if ok do return is_expr_eq(ex_a.left^, be_b.left^) && ex_a.op == be_b.op && is_expr_eq(ex_a.right^, be_b.right^)
		else do return false
	case Literal:
		lit_b, ok := b.(Literal)
		if ok do return is_lit_eq(ex_a, lit_b)
		else do return false
	case Identifier:
		unimplemented()
	}
	return false
}

is_lit_eq :: proc(a, b: Literal) -> bool {
	if type_of(a) == type_of(b) {
		switch x in a {
		case f32:
			return a.(f32) == b.(f32)
		case string:
			return a.(string) == b.(string)
		}
	}
	return false
}
