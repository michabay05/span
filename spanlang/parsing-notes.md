# Parsing notes

Notes taken from this video below at 00:42:43.
- Video link: https://youtu.be/fIPO4G42wYE?si=quH5TlLN5Ed9xqts

The examples below is pseudo-code in a c-like style without some punctuations

Some discussion about unary operators happens at 01:46:15.

---

```
parse_increasing_precedence(left, min_prec) {
    next = get_next_token()

    if !is_binary_operator(next) return left

    next_prec = get_precedence(next)

    if next_prec <= min_prec {
        return left
    } else {
        right = parse_expression(next_prec)
        return make_binary(left, to_operator(next), right)
    }
}
```

```
parse_expressions(min_prec) {
    left = parse_leaf()

    while true {
        node = parse_increasing_precedence(left, min_prec)
        if node == left { break }

        left = node
    }
}
```
