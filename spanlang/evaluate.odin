package spanlang

import "core:fmt"

Scope :: map[string]^Expr

eval_program :: proc(stmts: []Stmt, scope: ^Scope) {
	fmt.println("------------------------------------")
	for stmt in stmts {
		#partial switch st in stmt {
		case Def_Stmt:
			if st.ident.name not_in scope {
				scope[st.ident.name] = st.expr
				fmt.printfln("[INFO] added '%v' into scope", st.ident.name)
			} else {
				fmt.printfln("Redefinition of %v", st.ident.name)
			}
		case Expr_Stmt:
			expr := Expr(_eval_expr(st.expr, scope^))
			_dump_expr(&expr)
			fmt.println()
		case:
			fmt.eprintfln("[eval] TODO: handle %v\n", st)
			assert(false)
		}
	}
}

_eval_expr :: proc(expr: ^Expr, scope: Scope) -> Literal {
	switch ex in expr {
	case Literal: return ex
	case Identifier:
		value, ok := scope[ex.name]
		if ok {
			return _eval_expr(value, scope)
		} else {
			panic(fmt.tprintf("Undefined variable: '%s'", ex.name))
		}
	case Unary_Expr: unimplemented()
	case Binary_Expr:
		llit := _eval_expr(ex.left, scope)
		rlit := _eval_expr(ex.right, scope)
		return _eval_binary(ex.op, llit, rlit)
	}
	return nil
}

_eval_binary :: proc(op: Operator, left, right: Literal) -> Literal {
	#partial switch op {
	case .Add:
		#partial switch lv in left {
		case f32:
			rv, ok := right.(f32)
			assert(ok)
			return Literal(f32(lv + rv))
		case: unimplemented(fmt.tprintf("[eval] handle add op for type of %v", lv))
		}

	case:
		unimplemented(fmt.tprintf("[eval] handle op: %v", op))
	}
}
