<?hh // strict

type InnerShape = shape(?'inner_optional_field' => int);
type OuterShape = shape(?'outer_optional_field' => InnerShape);

function foo(OuterShape $shape): void {
  $shape['outer_optional_field']['inner_optional_field'] ?? 12;
}
