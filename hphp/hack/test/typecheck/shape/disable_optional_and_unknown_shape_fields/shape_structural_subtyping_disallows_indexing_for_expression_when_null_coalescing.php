<?hh // strict

type InnerShape = shape(?'inner_opt' => int);
type OuterShape = shape(?'outer_opt' => InnerShape);

function foo(bool $condition, OuterShape $shape): void {
  ($condition ? $shape['outer_opt'] : $shape['outer_opt'])['inner_opt'] ?? 12;
}
