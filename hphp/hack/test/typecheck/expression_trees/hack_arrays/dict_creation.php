<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

function test_dict_empty(): void {
  ExampleDsl`dict[]`;
}

function test_dict_in_expression_string_key(): void {
  ExampleDsl`(ExampleString $k, ExampleInt $v) ==> {
    $d = dict[$k => $v];
    $d;
  }`;
}

function test_dict_in_expression_int_key(): void {
  ExampleDsl`(ExampleInt $k, ExampleString $v) ==> {
    $d = dict[$k => $v];
    $d;
  }`;
}
