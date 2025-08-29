<?hh
<<file: __EnableUnstableFeatures('expression_tree_nested_bindings')>>
<<file: __EnableUnstableFeatures('expression_trees')>>

async function g(ExampleExpression<ExampleString> $s): Awaitable<ExampleExpression<ExampleInt>> {
  return ExampleDsl`1`;
}

async function f(): Awaitable<ExampleExpression<ExampleInt>> {
  return ExampleDsl`{
    $x = "hello";
    return ${await g(ExampleDsl`$x`)};
  }`;

}
