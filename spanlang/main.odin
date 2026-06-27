package spanlang

import "core:os"
import "core:fmt"
import "core:strings"
import "core:mem/virtual"

main :: proc() {
	buffer: [32*1024]byte
	arena: virtual.Arena
	err_ := virtual.arena_init_buffer(&arena, buffer[:])
	assert(err_ == nil)
	defer virtual.arena_destroy(&arena)
	context.allocator = virtual.arena_allocator(&arena)
	defer free_all(context.allocator)

    if len(os.args) != 2 {
        fmt.eprintfln("Usage: %s <INPUT.span>", os.args[0])
        os.exit(1)
    }

    data, err := os.read_entire_file(os.args[1], context.temp_allocator)
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
}
