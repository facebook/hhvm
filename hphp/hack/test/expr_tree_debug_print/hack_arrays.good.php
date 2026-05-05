<?hh
<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

function test_vec(): void {
  ExampleDsl`(vec<ExampleInt> $arr, ExampleInt $idx) ==> vec[$idx, 1, 2]`;
}

function test_dict_string_key(): void {
  ExampleDsl`(ExampleString $key, ExampleInt $val) ==> dict[$key => $val]`;
}

function test_dict_int_key(): void {
  ExampleDsl`(ExampleInt $key, ExampleString $val) ==> dict[$key => $val]`;
}

function test_vec_empty(): void {
  ExampleDsl`() ==> vec[]`;
}

function test_dict_empty(): void {
  ExampleDsl`() ==> dict[]`;
}

function test_keyset(): void {
  ExampleDsl`(ExampleInt $x) ==> keyset[$x, 1, 2]`;
}

function test_keyset_empty(): void {
  ExampleDsl`() ==> keyset[]`;
}

function test_keyset_string(): void {
  ExampleDsl`(ExampleString $s) ==> keyset[$s, "a"]`;
}
