# Operator Precedence

The precedence of Hack operators is shown in the table below.

Operators higher in the table have a higher precedence (binding more
tightly). Binary operators on the same row are evaluated according to their
associativity.

Operator | Description | Associativity
------------ | --------- | ---------
`\` | [Namespace separator](/docs/hack/source-code-fundamentals/namespaces)  | Left
`::` | [Scope resolution](/docs/hack/expressions-and-operators/scope-resolution)  | Left
`[]` | [Index resolution](/docs/hack/expressions-and-operators/subscript) for Hack Arrays and Strings | Left
`->`, `?->` | [Property selection](/docs/hack/expressions-and-operators/member-selection) and [null-safe property selection](/docs/hack/expressions-and-operators/member-selection#null-safe-member-access) | Left
`new` | [Object creation & Memory allocation](/docs/hack/expressions-and-operators/new) | None
`#`, `()` | [Enum class labels](/docs/hack/built-in-types/enum-class-label) and [Function calling](/docs/hack/functions/introduction) | Left
`clone` | Object cloning (shallowly, not deeply) | None
`readonly`, `await`, `++` `--` (postfix) | [Using readonly](/docs/hack/readonly/explicit-readonly-keywords), [Suspending an async function](/docs/hack/expressions-and-operators/await), and [Incrementing / Decrementing](/docs/hack/expressions-and-operators/incrementing-and-decrementing) (postfix) | Right
`(int)` `(float)` `(string)`, `**`, `@`, `++` `--` (prefix) | [Casting](/docs/hack/expressions-and-operators/casting), [Exponentiation](/docs/hack/expressions-and-operators/arithmetic#exponent), [Suppressing errors](/docs/hack/expressions-and-operators/error-control), and [Incrementing / Decrementing](/docs/hack/expressions-and-operators/incrementing-and-decrementing) (prefix) | Right
`is`, `as` `?as` |[Type checks / Type assertions](/docs/hack/expressions-and-operators/type-assertions) | Left
`!`, `~`, `+` `-` (one argument) | [Logical negation](/docs/hack/expressions-and-operators/logical-operators), [Bitwise negation](/docs/hack/expressions-and-operators/bitwise-operators#bitwise-negation), and [Unary Addition / Subtraction](/docs/hack/expressions-and-operators/arithmetic) | Right
`*` `/` `%` | [Multiplication, Division, and Modulo](/docs/hack/expressions-and-operators/arithmetic) | Left
`.`, `+` `-` (two arguments) | [String concatenation](/docs/hack/expressions-and-operators/string-concatenation) and [Addition / Subtraction](/docs/hack/expressions-and-operators/arithmetic) | Left
`<<` `>>` | [Bitwise shifting](/docs/hack/expressions-and-operators/bitwise-operators) (left and right) | Left
`<` `<=` `>` `>=`, `<=>` | [Comparison operators](/docs/hack/expressions-and-operators/comparisons) and [Spaceship operator](/docs/hack/expressions-and-operators/comparisons/#the-spaceship-operator) | None
`===` `!==` `==` `!=` | [Equality operators](/docs/hack/expressions-and-operators/equality) | None
`&` | [Bitwise AND](/docs/hack/expressions-and-operators/bitwise-operators) | Left
`^` | [Bitwise XOR](/docs/hack/expressions-and-operators/bitwise-operators) | Left
`\|` | [Bitwise OR](/docs/hack/expressions-and-operators/bitwise-operators) | Left
`&&` | [Logical AND](/docs/hack/expressions-and-operators/logical-operators) | Left
`\|\|` | [Logical OR](/docs/hack/expressions-and-operators/logical-operators) | Left
`??` | [Coalesce operator](/docs/hack/expressions-and-operators/coalesce) | Right
`?` `:`, `?:` | [Ternary evaluation](/docs/hack/expressions-and-operators/ternary) and [Elvis operator](/docs/hack/expressions-and-operators/ternary#elvis-operator) | Left
`\|>` | [Pipe / Chain function calls](/docs/hack/expressions-and-operators/pipe) | Left
`=` `+=` `-=` `.=` `*=` `/=` `%=` `<<=` `>>=` `&=` `^=` `\|=`, `??=` | [Assignment operators](/docs/hack/expressions-and-operators/assignment) and [Coalescing assignment operator](/docs/hack/expressions-and-operators/coalesce#coalescing-assignment-operator) | Right
`echo` | [Write to standard output](/docs/hack/expressions-and-operators/echo) | Right
`include` `require` | [Include or Require a script](/docs/hack/source-code-fundamentals/script-inclusion)| Left
