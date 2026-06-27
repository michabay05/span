package spancli

import "core:bufio"
import "core:fmt"
import "core:os"
import "core:strings"
import spl "spanlang"

main :: proc() {
	program, args := os.args[0], os.args[1:]
	if len(args) < 1 do usage(program)

	switch args[0] {
	case "r":
		if len(args) != 2 {
			fmt.println("ERROR: Provide a input file to run")
			usage(program)
		}
		run_file(args[1])

	case "i":
		start_repl()
	}
}

run_file :: proc(filepath: string) {
	data, err := os.read_entire_file(filepath, context.allocator)
    if err != nil {
        fmt.eprintln(err)
        return
    }
    content := strings.trim_space(string(data))
    fmt.println(content)

    tokens: [dynamic]spl.Token
    defer delete(tokens)
    spl.lex_content(content, &tokens)
    for token, i in tokens {
        fmt.printfln("[% 3d] %15s\t%s", i, token.kind, token.literal)
    }

    stmts: [dynamic]spl.Stmt
    defer delete(stmts)
    spl.parse_program(tokens[:], &stmts)

    for stmt in stmts {
    	spl.dump_stmt(stmt)
    }

    spl.eval_program(stmts[:])
}

start_repl :: proc() {
	fmt.println("span repl")

	scanner: bufio.Scanner
	stdin := os.to_stream(os.stdin)
	bufio.scanner_init(&scanner, stdin)

	tokens: [dynamic]spl.Token
	defer delete(tokens)
	stmts: [dynamic]spl.Stmt
	defer delete(stmts)
	for {
		fmt.print("> ")
		if !bufio.scanner_scan(&scanner) {
			fmt.println("\n\nLeaving...Goodbye!")
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

usage :: proc(program: string) {
	fmt.printfln("Usage: %s <c|r> [OPTIONS]", program)
	fmt.println("Options:")
	fmt.println("    r <file>   run file")
	fmt.println("    i          start interactive repl")
}
