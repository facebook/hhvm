# Splicing

Expression trees support "splicing", where you insert one expression tree into another.

```hack
<<file:__EnableUnstableFeatures('expression_trees')>>

function splicing_example(bool $b): ExampleExpression<ExampleString> {
  $name = $b ? ExampleDsl`"world"` : ExampleDsl`"universe"`;
  return ExampleDsl`"Hello, ".${$name}."!"`;
}
```

This allows you to build different expressions based on runtime values. DSL expressions are not evaluated, but the value in the `${...}` is evaluated and inserted into the expression tree.

The above example is equivalent to this:

```hack
<<file:__EnableUnstableFeatures('expression_trees')>>

function splicing_example2(bool $b): ExampleExpression<ExampleString> {
  return $b ? ExampleDsl`"Hello, "."world"."!"` : ExampleDsl`"Hello, "."universe"."!"`;
}
```

## Limitations

Every DSL expression must be valid in isolation. You cannot use free
variables in expression trees, even when splicing.

```hack error
$var = ExampleDsl`$x`; // type error: $x is not defined
ExampleDsl`($x) ==> { return ${$var}; }`;
```

## Implementing Splicing

A visitor needs to support the `splice` method to allow splicing. The `splice` method is passed the inner expression tree value, along with a unique key that may be used for caching visitor results.

```hack no-extract
final class MyDsl {
  public function splice(
    ?ExprPos $_pos,
    string $_key,
    Spliceable<MyDsl, MyDslAst, mixed> $code,
  ): MyDslAst {
    return $code->visit($this);
  }

  // ...
}
```

For more information, see: [Defining Visitors: Spliceable Types](/hack/expression-trees/defining-dsls#spliceable-types).
