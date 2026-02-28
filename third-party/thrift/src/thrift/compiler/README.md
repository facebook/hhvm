# Facebook Thrift Compiler

This directory contains a Thrift compiler library and the driver program,
`thrift1`, that parses `.thrift` files and generates code in different
languages.

## Directory Layout

* `ast`: abstract syntax tree
* `codemod`: codemods for Thrift code
* `detail`: implementation details shared between compiler phases
* `generate`: code generators for target languages
* `parse`: lexer and parser
* `sema`: semantic analyzer
* `test`: compiler tests
* `whisker`: templating engine used in generators
