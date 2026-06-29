package spanlang

import "core:bufio"
import "core:fmt"
import "core:os"
import "core:strings"
import "core:mem/virtual"

main :: proc() {
	program, args := os.args[0], os.args[1:]
	if len(args) < 1 {
		usage(program)
		return
	}

	buffer: [32*1024]byte
	arena: virtual.Arena
	err_ := virtual.arena_init_buffer(&arena, buffer[:])
	assert(err_ == nil)
	defer virtual.arena_destroy(&arena)
	context.allocator = virtual.arena_allocator(&arena)
	defer free_all(context.allocator)

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

    tokens: [dynamic]Token
    defer delete(tokens)
    lex_content(content, &tokens)
    for token, i in tokens {
        fmt.printfln("[% 3d] %15s\t%s", i, token.kind, token.literal)
    }

    stmts: [dynamic]Stmt
    defer delete(stmts)
    parse_program(tokens[:], &stmts)

    for stmt in stmts {
    	dump_stmt(stmt)
    }

    scope := make(Scope)
    defer delete(scope)
    eval_program(stmts[:], &scope)
}

start_repl :: proc() {
	fmt.println("span repl")

	scanner: bufio.Scanner
	stdin := os.to_stream(os.stdin)
	bufio.scanner_init(&scanner, stdin)

	tokens: [dynamic]Token
	defer delete(tokens)
	stmts: [dynamic]Stmt
	defer delete(stmts)
	scope := make(Scope)
    defer delete(scope)
	for {
		fmt.print("> ")
		if !bufio.scanner_scan(&scanner) {
			fmt.println("\n\nLeaving...Goodbye!")
			break
		}

		line := bufio.scanner_text(&scanner)
		clear(&tokens)
		lex_content(line, &tokens)
		// TODO: This is just a hack for now
		if tokens[len(tokens) - 1].kind != .Semicolon {
			append(&tokens, Token{ .Semicolon, ";" })
		}

		clear(&stmts)
		parse_program(tokens[:], &stmts)
		for stmt in stmts {
			dump_stmt(stmt)
		}

    	eval_program(stmts[:], &scope)
	}
}

usage :: proc(program: string) {
	fmt.printfln("Usage: %s <c|r> [OPTIONS]", program)
	fmt.println("Options:")
	fmt.println("    r <file>   run file")
	fmt.println("    i          start interactive repl")
}
