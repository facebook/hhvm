<?hh
<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_coalesce_operator')>>

function f(): void {
  $x = ExampleDsl`null`;
  $y = ExampleDsl`2`;
  ExampleDsl`null ?? null `;
  ExampleDsl`null ?? 1`;
  ExampleDsl`null ?? ${$y}`;
  ExampleDsl`${$x} ?? ${$y}`;
  ExampleDsl`${$y} ?? ${$x}`;
  // The coalesce operator is right-associative
  ExampleDsl`${$x} ?? null ?? 1`;
  ExampleDsl`(${$x} ?? null) ?? 1`;
  ExampleDsl`${$x} ?? (null ?? 1)`;
  // The addition operator has a higher precedence than the coalesce operator
  ExampleDsl`(${$x} ?? ${$y}) + 1`;
}
