<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

function test_vec_empty(): void {
  ExampleDsl`vec[]`;
}

function test_vec_nested(): void {
  ExampleDsl`vec[vec[1, 2], vec[3, 4]]`;
}

function test_vec_in_expression(): void {
  ExampleDsl`(ExampleInt $x) ==> {
    $v = vec[$x, 1, 2];
    $v;
  }`;
}
