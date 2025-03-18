<?hh
<<file: __EnableUnstableFeatures('expression_tree_nested_bindings')>>
<<file: __EnableUnstableFeatures('expression_trees')>>

async function g(ExampleDslExpression<ExampleString> $s): Awaitable<ExampleDslExpression<ExampleInt>> {
  return ExampleDsl`1`;
}

async function f(): Awaitable<ExampleDslExpression<ExampleInt>> {
  return ExampleDsl`{
    $x = "hello";
    return ${await g(ExampleDsl`$x`)};
  }`;

}
