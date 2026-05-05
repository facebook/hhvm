<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

function test_keyset_empty(): void {
  ExampleDsl`keyset[]`;
}

function test_keyset_in_expression(): void {
  ExampleDsl`(ExampleInt $x) ==> {
    $v = keyset[$x, 1, 2];
    $v;
  }`;
}

function test_keyset_string_in_expression(): void {
  ExampleDsl`(ExampleString $s) ==> {
    $v = keyset[$s, "a", "b"];
    $v;
  }`;
}
