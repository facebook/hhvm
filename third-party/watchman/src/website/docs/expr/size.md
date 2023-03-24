---
title: size
section: Expression Terms
---

_Since version 3.1_

The size term allows the size of an existing file (not deleted) to be evaluated
using simple relational operators as described in the table below.

The size term must always be an array with 3 elements:

     ["size", "gt", 0]

The second parameter describes the relational operator and the third parameter
is the integer _operand_ to compare against. The example above evaluates to
`true` if the file exists and its size is greater than zero.

Possible relational operators are:

| Operator | Meaning               | Result                                      |
| -------- | --------------------- | ------------------------------------------- |
| `eq`     | Equal                 | `true` if file exists and `size == operand` |
| `ne`     | Not Equal             | `true` if file exists and `size != operand` |
| `gt`     | Greater Than          | `true` if file exists and `size > operand`  |
| `ge`     | Greater Than Or Equal | `true` if file exists and `size >= operand` |
| `lt`     | Less Than             | `true` if file exists and `size < operand`  |
| `le`     | Less Than Or Equal    | `true` if file exists and `size <= operand` |
