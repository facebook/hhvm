Expression tree DSLs can use most of the expression and statement syntax in Hack.

For a full list of supported syntax, see also [expr_tree.hhi](https://github.com/facebook/hhvm/blob/master/hphp/hack/test/hhi/expr_tree.hhi).

## Literals

| Example | Visitor Runtime                     | Typing                |
|---------|-------------------------------------|-----------------------|
| `true`  | `$v->visitBool($position, true)`    | `MyDsl::boolType()`   |
| `1`     | `$v->visitInt($position, 1)`        | `MyDsl::intType()`    |
| `1.23`  | `$v->visitFloat($position, 1.23)`   | `MyDsl::floatType()`  |
| `"foo"` | `$v->visitString($position, "foo")` | `MyDsl::stringType()` |
| `null`  | `$v->visitString($position)`        | `MyDsl::nullType()`   |
| N/A     | N/A                                 | `MyDsl::voidType()`   |

## Binary Operators

Operator names are based on the appearance of the symbol, as different
DSLs may choose different semantics for an operator.

| Example     | Visitor Runtime                                                   | Typing                                    |
|-------------|-------------------------------------------------------------------|-------------------------------------------|
| `$x + $y`   | `$v->visitBinop($position, ..., '__plus', ...)`                   | `__plus` method on `$x`                   |
| `$x - $y`   | `$v->visitBinop($position, ..., '__minus', ...)`                  | `__minus` method on `$x`                  |
| `$x * $y`   | `$v->visitBinop($position, ..., '__star', ...)`                   | `__star` method on `$x`                   |
| `$x / $y`   | `$v->visitBinop($position, ..., '__slash', ...)`                  | `__slash` method on `$x`                  |
| `$x % $y`   | `$v->visitBinop($position, ..., '__percent', ...)`                | `__percent` method on `$x`                |
| `$x && $y`  | `$v->visitBinop($position, ..., '__ampamp', ...)`                 | `__ampamp` method on `$x`                 |
| `$x \|\| $y`  | `$v->visitBinop($position, ..., '__barbar', ...)`                 | `__barbar` method on `$x`                 |
| `$x < $y`   | `$v->visitBinop($position, ..., '__lessThan', ...)`               | `__lessThan` method on `$x`               |
| `$x <= $y`  | `$v->visitBinop($position, ..., '__lessThanEqual', ...)`           | `__lessThanEqual` method on `$x`           |
| `$x > $y`   | `$v->visitBinop($position, ..., '__greaterThan', ...)`            | `__greaterThan` method on `$x`            |
| `$x >= $y`  | `$v->visitBinop($position, ..., '__greaterThanEqual', ...)`       | `__greaterThanEqual` method on `$x`       |
| `$x === $y` | `$v->visitBinop($position, ..., '__tripleEquals', ...)`           | `__tripleEquals` method on `$x`           |
| `$x !== $y` | `$v->visitBinop($position, ..., '__notTripleEquals', ...)`        | `__notTripleEquals` method on `$x`        |
| `$x . $y`   | `$v->visitBinop($position, ..., '__dot', ...)`                    | `__dot` method on `$x`                    |
| `$x & $y`   | `$v->visitBinop($position, ..., '__amp', ...)`                    | `__amp` method on `$x`                    |
| `$x \| $y`   | `$v->visitBinop($position, ..., '__bar', ...)`                    | `__bar` method on `$x`                    |
| `$x ^ $y`   | `$v->visitBinop($position, ..., '__caret', ...)`                  | `__caret` method on `$x`                  |
| `$x << $y`  | `$v->visitBinop($position, ..., '__lessThanLessThan', ...)`       | `__lessThanLessThan` method on `$x`       |
| `$x >> $y`  | `$v->visitBinop($position, ..., '__greaterThanGreaterThan', ...)` | `__greaterThanGreaterThan` method on `$x` |

## Ternary Operators

| Example        | Visitor Runtime                             | Typing                   |
|----------------|---------------------------------------------|--------------------------|
| `$x ? $y : $z` | `$v->visitTernary($position, ..., ..., ...)` | `$x->__bool() ? $y : $z` |

## Unary Operators

| Example | Visitor Runtime                                            | Typing                             |
|---------|------------------------------------------------------------|------------------------------------|
| `!$x`   | `$v->visitUnop($position, ..., '__exclamationMark')` | `__exclamationMark` method on `$x` |
| `-$x`   | `$v->visitUnop($position, ..., '__negate')`          | `__negate` method on `$x`          |
| `~$x`   | `$v->visitUnop($position, ..., '__tilde')`           | `__tilde` method on `$x`           |
| `$x++` | `$v->visitUnop($position, ..., '__postfixPlusPlus')`  | `__postfixPlusPlus` method on `$x` |
| `$x--` | `$v->visitUnop($position, ..., '__postfixMinusMinus')` | `__postfixMinusMinus` method on `$x` |


## Local Variables

| Example   | Visitor Runtime                        | Typing              |
|-----------|----------------------------------------|---------------------|
| `$x`      | `$v->visitLocal($position, '$x')`      | Same as normal Hack |

You can see here that the visitor runtime does not know the type of `$x`, it
just sees a call to `visitLocal` with the variable name as a string
`'$x'`.

Note that expression trees do not allow free variables. You can only
use local variables that have been previously assigned or introduced
with a lambda.

## Lambdas

| Example           | Visitor Runtime                                   | Typing              |
|-------------------|---------------------------------------------------|---------------------|
| `(Foo $x) ==> $y` | `$v->visitLambda($position, vec['$x'], vec[...])` | Same as normal Hack |

Note that the visitor runtime does not see the type of `$x`.

## Statements

| Example                     | Visitor Runtime                                              | Typing                                |
|-----------------------------|--------------------------------------------------------------|---------------------------------------|
| `$x = $y;`                  | `$v->visitAssign($position, ..., ...)`                       | Same as normal Hack                   |
| `return $x;`                | `$v->visitReturn($position, ...)`                            | Same as normal Hack                   |
| `return;`                   | `$v->visitReturn($position, null)`                           | Same as normal Hack                   |
| `if (...) {...} else {...}` | `$v->visitIf($position, ..., ..., ...)`                      | `if (...->__bool()) {...} else {...}` |
| `while (...) {...}`         | `$v->visitWhile($position, ..., vec[...])`                   | `while (...->__bool()) {...}`         |
| `for (...; ...; ...) {...}` | `$v->visitFor($position, vec[...], ..., vec[...], vec[...])` | `for (...; ...->__bool(); ...) {...}` |
| `break;`                    | `$v->visitBreak($position)`                                  | Same as normal Hack                   |
| `continue;`                 | `$v->visitContinue($position)`                               | Same as normal Hack                   |

## Calls

| Example         | Visitor Runtime                                                                     | Typing                            |
|-----------------|-------------------------------------------------------------------------------------|-----------------------------------|
| `foo(...)`      | `$v->visitCall($position, $v->visitGlobalFunction($position, foo<>), vec[...])`     | `MyDsl::symbolType(foo<>)()`      |
| `Foo::bar(...)` | `$v->visitCall($position, $v->visitStaticMethod($position, Foo::bar<>), vec[...])`  | `MyDsl::symbolType(Foo::bar<>)()` |

Note that the function or method must have a Hack definition, so the typechecker can verify that it's being called with appropriate arguments.

## XHP Literals

| Example              | Visitor Runtime                                             | Typing               |
|----------------------|-------------------------------------------------------------|----------------------|
| `<foo ...>...</foo>` | `$v->visitXhp($position, :foo::class, dict[...], vec[...])` | `<foo ...>...</foo>` |


## Property Access

| Example      | Visitor Runtime                                  | Typing       |
|--------------|--------------------------------------------------|--------------|
| `(...)->foo` | `$v->visitPropertyAccess($position, ..., 'foo')` | `(...)->foo` |

## Splicing

| Example      | Visitor Runtime                                  | Typing       |
|--------------|--------------------------------------------------|--------------|
| `${$x}` | `$v->splice($position, 'key0', $x)` | Extract the inferred type out of the third type argument of `Spliceable` |

## Unsupported Features

Expression trees may only contain expressions between the backticks.

```hack error
MyDsl`1`; // OK: 1 is an expression
MyDsl`while(true) {}`; // Bad: statement
MyDsl`() ==> { while true() {} }`; // OK: statements are allowed in lambdas
MyDsl`class Foo {}`; // Bad: top-level declaration.
```

## Expression Tree Blocks

There are times when it is desirable to include statements as part of an expression tree. One way to accomplish this is to create a lambda that is immediately invoked.

```hack
<<file:__EnableUnstableFeatures('expression_trees')>>

function example1(): void {
  $num = ExampleDsl`1 + 1`;
  ExampleDsl`() ==> {
    $n = ${$num};
    return $n + $n;
  }()`;
}
```

This can be rewritten using expression tree blocks, eliminating the need to create a lambda and invoke it.

```hack
<<file:__EnableUnstableFeatures('expression_trees')>>

function example2(): void {
  $num = ExampleDsl`1 + 1`;
  ExampleDsl`{
    $n = ${$num};
    return $n + $n;
  }`;
}
```

Note that expression tree blocks are syntactic sugar. It will be expanded to the longer form for both type checking and the visitor runtime.
