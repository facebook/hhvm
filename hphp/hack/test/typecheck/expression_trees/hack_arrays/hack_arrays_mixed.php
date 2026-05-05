<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

function test_vec_in_dict_string_key(): void {
  ExampleDsl`(ExampleInt $x) ==> {
    $v = vec[$x, 1];
    $d = dict["a" => $x];
    $d;
  }`;
}

function test_vec_in_dict_int_key(): void {
  ExampleDsl`(ExampleInt $x) ==> {
    $v = vec[$x, 1];
    $d = dict[0 => $x];
    $d;
  }`;
}

function test_keyset_in_dict_string_key(): void {
  ExampleDsl`(ExampleInt $x) ==> {
    $k = keyset[$x, 1];
    $d = dict["a" => $k];
    $d;
  }`;
}
