# `span` language

## Purpose
Programming languages have often tended towards becoming more general purpose.
This project is an experiment intentionally going in the opposite direction:
domain-specific. Having domain-specific languages allows a language to
specialize in its specific field of domain; languages like Markdown, SQL and
Latex provide some indications of this idea in the wild. Although one can write
a research paper in Python, rarely does someone decide to do so. People can
choose to write the contents of their blogs in HTML; however, the terseness of
Markdown remains undeniable. However, in the context of programmatic animation
libraries, general purpose programming languages are twisted and bent to
generate programmatic animations. My goal for this project is to create a
programming language purpose-built for programmatic video animations. While
there is no guarantee of success, the outcome of this project will most likely
be fruitful...if not to the general public, most definitely to me.

## Quickstart
In order to run this project, you need to have an `odin` compiler.
```
$ odin build spanlang
$ ./spanlang.bin i
```
Running this should start a basic REPL to experiment with span.

## Resources
Here are some resources I relied on to get me through this project.
- [Writing an Interpreter in Go](https://interpreterbook.com/)
- [Writing a Compiler in Go](https://compilerbook.com/)
- [Talk about between `jblow` and `cmuratori` about the ease of parsing](https://youtu.be/fIPO4G42wYE?si=quH5TlLN5Ed9xqts)
    - Look at `spanlang/parsing-notes.md` for my takeaways from it
