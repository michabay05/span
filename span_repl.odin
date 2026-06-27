package span_repl

import "core:bufio"
import "core:fmt"
import "core:os"
import spl "spanlang"

main :: proc() {
	scanner: bufio.Scanner

	stdin := os.to_stream(os.stdin)
	bufio.scanner_init(&scanner, stdin)

	tokens: [dynamic]spl.Token
	stmts: [dynamic]spl.Stmt
	for {
		if !bufio.scanner_scan(&scanner) {
			fmt.println("Leaving...Goodbye!")
			break
		}

		line := bufio.scanner_text(&scanner)
		clear(&tokens)
		spl.lex_content(line, &tokens)
		// TODO: This is just a hack for now
		if tokens[len(tokens) - 1].kind != .Semicolon {
			append(&tokens, spl.Token{ .Semicolon, ";" })
		}

		clear(&stmts)
		spl.parse_program(tokens[:], &stmts)
		for stmt in stmts {
			spl.dump_stmt(stmt)
		}

		spl.eval_program(stmts[:])
	}
}
