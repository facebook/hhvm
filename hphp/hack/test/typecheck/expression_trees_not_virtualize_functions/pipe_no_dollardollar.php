<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

// Note that there is no usage of the $$ here.
// The check for assignment in pipe only checks to see whether $$ is in scope,
// not whether it is being used
// For assignments to occur in expression trees,
// a splice will cause an evaluation of an assignment
function test(ExprTree<ExampleDsl, ExampleDsl::TAst, ExampleInt> $x): void {
  1 |> ExampleDsl`${$x}`;
}
