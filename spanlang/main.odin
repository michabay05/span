package spanlang

import "core:os"
import "core:fmt"
import "core:strings"

main :: proc() {
    data, err := os.read_entire_file("spanlang/simple.span", context.allocator)
    if err != nil {
        fmt.eprintln(err)
        return
    }
    content := strings.trim_space(string(data))
    fmt.println(content)

    lex_content(content)
}
