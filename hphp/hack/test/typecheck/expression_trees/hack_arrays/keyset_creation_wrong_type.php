<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

function test_keyset_wrong_element_type(): void {
  ExampleDsl`{
    $f = (keyset<ExampleInt> $v) ==> 3;
    $f(keyset["a", "b"]);
  }`;
}

function test_keyset_mixed_element_types(): void {
  ExampleDsl`{
    $f = (keyset<ExampleInt> $v) ==> 3;
    $f(keyset[1, "a"]);
  }`;
}

function test_keyset_string_wrong_element_type(): void {
  ExampleDsl`{
    $f = (keyset<ExampleString> $v) ==> 3;
    $f(keyset[1, 2]);
  }`;
}
