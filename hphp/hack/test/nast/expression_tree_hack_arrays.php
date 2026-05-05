<?hh

<<file: __EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

function test_vec(): void {
  ExampleDsl`(ExampleInt $x) ==> vec[$x, 1]`;
}

function test_dict(): void {
  ExampleDsl`(ExampleString $k, ExampleInt $v) ==> dict[$k => $v]`;
}

function test_dict_int_key(): void {
  ExampleDsl`(ExampleInt $k, ExampleString $v) ==> dict[$k => $v]`;
}

function test_keyset_int(): void {
  ExampleDsl`(ExampleInt $x) ==> keyset[$x, 1]`;
}

function test_keyset_string(): void {
  ExampleDsl`(ExampleString $s) ==> keyset[$s, "a"]`;
}
