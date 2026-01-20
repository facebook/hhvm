# Operator Precedence

The precedence of Hack operators is shown in the table below.

Operators higher in the table have a higher precedence (binding more
tightly). Binary operators on the same row are evaluated according to their
associativity.

Operator | Description | Associativity
------------ | --------- | ---------
`\` | [Namespace separator](/hack/source-code-fundamentals/namespaces)  | Left
`::` | [Scope resolution](/hack/expressions-and-operators/scope-resolution)  | Left
`[]` | [Index resolution](/hack/expressions-and-operators/subscript) for Hack Arrays and Strings | Left
`->`, `?->` | [Property selection](/hack/expressions-and-operators/member-selection) and [null-safe property selection](/hack/expressions-and-operators/member-selection#null-safe-member-access) | Left
`new` | [Object creation & Memory allocation](/hack/expressions-and-operators/new) | None
`#`, `()` | [Enum class labels](/hack/built-in-types/enum-class-label) and [Function calling](/hack/functions/introduction) | Left
`clone` | Object cloning (shallowly, not deeply) | None
`readonly`, `await`, `++` `--` (postfix) | [Using readonly](/hack/readonly/explicit-readonly-keywords), [Suspending an async function](/hack/expressions-and-operators/await), and [Incrementing / Decrementing](/hack/expressions-and-operators/incrementing-and-decrementing) (postfix) | Right
`(int)` `(float)` `(string)`, `**`, `@`, `++` `--` (prefix) | [Casting](/hack/expressions-and-operators/casting), [Exponentiation](/hack/expressions-and-operators/arithmetic#exponent), [Suppressing errors](/hack/expressions-and-operators/error-control), and [Incrementing / Decrementing](/hack/expressions-and-operators/incrementing-and-decrementing) (prefix) | Right
`is`, `as` `?as` |[Type checks / Type assertions](/hack/expressions-and-operators/type-assertions) | Left
`!`, `~`, `+` `-` (one argument) | [Logical negation](/hack/expressions-and-operators/logical-operators), [Bitwise negation](/hack/expressions-and-operators/bitwise-operators#bitwise-negation), and [Unary Addition / Subtraction](/hack/expressions-and-operators/arithmetic) | Right
`*` `/` `%` | [Multiplication, Division, and Modulo](/hack/expressions-and-operators/arithmetic) | Left
`.`, `+` `-` (two arguments) | [String concatenation](/hack/expressions-and-operators/string-concatenation) and [Addition / Subtraction](/hack/expressions-and-operators/arithmetic) | Left
`<<` `>>` | [Bitwise shifting](/hack/expressions-and-operators/bitwise-operators) (left and right) | Left
`<` `<=` `>` `>=`, `<=>` | [Comparison operators](/hack/expressions-and-operators/comparisons) and [Spaceship operator](/hack/expressions-and-operators/comparisons/#the-spaceship-operator) | None
`===` `!==` `==` `!=` | [Equality operators](/hack/expressions-and-operators/equality) | None
`&` | [Bitwise AND](/hack/expressions-and-operators/bitwise-operators) | Left
`^` | [Bitwise XOR](/hack/expressions-and-operators/bitwise-operators) | Left
`\|` | [Bitwise OR](/hack/expressions-and-operators/bitwise-operators) | Left
`&&` | [Logical AND](/hack/expressions-and-operators/logical-operators) | Left
`\|\|` | [Logical OR](/hack/expressions-and-operators/logical-operators) | Left
`??` | [Coalesce operator](/hack/expressions-and-operators/coalesce) | Right
`?` `:`, `?:` | [Ternary evaluation](/hack/expressions-and-operators/ternary) and [Elvis operator](/hack/expressions-and-operators/ternary#elvis-operator) | Left
`\|>` | [Pipe / Chain function calls](/hack/expressions-and-operators/pipe) | Left
`=` `+=` `-=` `.=` `*=` `/=` `%=` `<<=` `>>=` `&=` `^=` `\|=`, `??=` | [Assignment operators](/hack/expressions-and-operators/assignment) and [Coalescing assignment operator](/hack/expressions-and-operators/coalesce#coalescing-assignment-operator) | Right
`echo` | [Write to standard output](/hack/expressions-and-operators/echo) | Right
`include` `require` | [Include or Require a script](/hack/source-code-fundamentals/script-inclusion)| Left
