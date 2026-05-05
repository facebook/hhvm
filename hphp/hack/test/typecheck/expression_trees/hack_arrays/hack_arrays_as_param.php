<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

function test_vec_as_lambda_param(): void {
  ExampleDsl`(vec<ExampleInt> $v) ==> $v`;
}

function test_dict_as_lambda_param(): void {
  ExampleDsl`(dict<ExampleString, ExampleInt> $d) ==> $d`;
}

function test_vec_roundtrip(): void {
  ExampleDsl`{
    $f = (vec<ExampleInt> $v) ==> $v;
    $f(vec[1, 2, 3]);
  }`;
}

function test_dict_roundtrip(): void {
  ExampleDsl`{
    $f = (dict<ExampleString, ExampleInt> $d) ==> $d;
    $f(dict["a" => 1, "b" => 2]);
  }`;
}

function test_keyset_as_lambda_param(): void {
  ExampleDsl`(keyset<ExampleInt> $k) ==> $k`;
}

function test_keyset_roundtrip(): void {
  ExampleDsl`{
    $f = (keyset<ExampleInt> $k) ==> $k;
    $f(keyset[1, 2, 3]);
  }`;
}

function test_dict_int_key_as_lambda_param(): void {
  ExampleDsl`(dict<ExampleInt, ExampleString> $d) ==> $d`;
}

function test_dict_int_key_roundtrip(): void {
  ExampleDsl`{
    $f = (dict<ExampleInt, ExampleString> $d) ==> $d;
    $f(dict[0 => "a", 1 => "b"]);
  }`;
}

function test_keyset_string_as_lambda_param(): void {
  ExampleDsl`(keyset<ExampleString> $k) ==> $k`;
}

function test_keyset_string_roundtrip(): void {
  ExampleDsl`{
    $f = (keyset<ExampleString> $k) ==> $k;
    $f(keyset["a", "b", "c"]);
  }`;
}
