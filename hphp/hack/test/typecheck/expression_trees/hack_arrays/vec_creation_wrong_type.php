<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

function test_vec_wrong_element_type(): void {
  ExampleDsl`{
    $f = (vec<ExampleInt> $v) ==> 3;
    $f(vec["a", "b"]);
  }`;
}

function test_vec_mixed_element_types(): void {
  ExampleDsl`{
    $f = (vec<ExampleInt> $v) ==> 3;
    $f(vec[1, "a"]);
  }`;
}
