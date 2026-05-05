<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

function test_dict_wrong_value_type(): void {
  ExampleDsl`{
    $f = (dict<ExampleString, ExampleInt> $d) ==> 3;
    $f(dict["a" => "b"]);
  }`;
}

function test_dict_wrong_key_type(): void {
  ExampleDsl`{
    $f = (dict<ExampleString, ExampleInt> $d) ==> 3;
    $f(dict[0 => 1]);
  }`;
}

function test_dict_int_key_wrong_key_type(): void {
  ExampleDsl`{
    $f = (dict<ExampleInt, ExampleString> $d) ==> 3;
    $f(dict["a" => "b"]);
  }`;
}
