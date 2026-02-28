<?hh

<<file:__EnableUnstableFeatures('expression_trees')>>

function redundant_option_mixed_expression_trees(): void {
  ExampleDsl`(?mixed $x) ==> {}`;
}
