package spanlang

import "core:fmt"

Scope :: map[string]Literal

eval_program :: proc(stmts: []Stmt) {
	fmt.println("------------------------------------")
	scope: Scope
	for stmt in stmts {
		#partial switch st in stmt {
		case Def_Stmt:
			fmt.printfln("%v", st)
		case Expr_Stmt:
			lit := _eval_expr(st.expr)
			fmt.println(lit)
		case:
			fmt.eprintfln("[eval] TODO: handle %v\n", st)
			assert(false)
		}
	}
}

_eval_expr :: proc(expr: ^Expr) -> Literal {
	switch ex in expr^ {
	case Identifier:
		unimplemented()
	case Literal:
		return ex
	case Infix_Expr:
		return _eval_Infix_expr(ex)
	}
	unreachable()
}

_eval_Infix_expr :: proc(be: Infix_Expr) -> Literal {
	left := _eval_expr(be.left)
	right := _eval_expr(be.right)

	#partial switch be.op {
	case .Add:
		switch lv in left {
		case f32:
			rv, ok := right.(f32)
			assert(ok, "left and right value are not of the same type")
			return Literal(f32(lv + rv))
		case string:
			fmt.eprintln("[eval] handle binary expr for string literals")
			unreachable()
		case Vector:
			fmt.eprintln("[eval] handle add op for vector literals")
			unreachable()
		}

	case .Sub:
		switch lv in left {
		case f32:
			rv, ok := right.(f32)
			assert(ok, "left and right value are not of the same type")
			return Literal(f32(lv - rv))
		case string:
			fmt.eprintfln("[eval] undefined operations for strings (op=%v)", be.op)
			unreachable()
		case Vector:
			fmt.eprintln("[eval] handle sub op for vector literals")
			unreachable()
		}

	case .Multiply:
		switch lv in left {
		case f32:
			rv, ok := right.(f32)
			assert(ok, "left and right value are not of the same type")
			return Literal(f32(lv * rv))
		case string:
			fmt.eprintfln("[eval] undefined operations for strings (op=%v)", be.op)
			unreachable()
		case Vector:
			fmt.eprintln("[eval] handle mult op for vector literals")
			unreachable()
		}

	case .Divide:
		switch lv in left {
		case f32:
			rv, ok := right.(f32)
			assert(ok, "left and right value are not of the same type")
			if rv == 0.0 {
				panic("ERROR: div by zero; (a / 0) is not defined")
			}
			return Literal(f32(lv / rv))
		case string:
			fmt.eprintfln("[eval] undefined operations for strings (op=%v)", be.op)
			unreachable()
		case Vector:
			fmt.eprintln("[eval] handle mult op for vector literals")
			unreachable()
		}

	case:
		fmt.eprintfln("[eval] handle other binary operations (like %v)", be.op)
		unreachable()
	}
	unreachable()
}
