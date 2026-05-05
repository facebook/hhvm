<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

// Type inference flow catches mismatches — all should error.

// Inferred vec<int> passed to vec<string> param.
function test_vec_inferred_wrong_param(): void {
  ExampleDsl`{
    $f = (vec<ExampleString> $v) ==> $v;
    $v = vec[1, 2, 3];
    $f($v);
  }`;
}

// Inferred dict<string, int> passed to dict<string, string> param.
function test_dict_inferred_wrong_value_param(): void {
  ExampleDsl`{
    $f = (dict<ExampleString, ExampleString> $d) ==> $d;
    $d = dict["a" => 1];
    $f($d);
  }`;
}

// Inferred dict<string, int> passed to dict<int, int> param (wrong key).
function test_dict_inferred_wrong_key_param(): void {
  ExampleDsl`{
    $f = (dict<ExampleInt, ExampleInt> $d) ==> $d;
    $d = dict["a" => 1];
    $f($d);
  }`;
}

// Inferred keyset<int> passed to keyset<string> param.
function test_keyset_inferred_wrong_param(): void {
  ExampleDsl`{
    $f = (keyset<ExampleString> $k) ==> $k;
    $k = keyset[1, 2];
    $f($k);
  }`;
}

// Lambda returns vec<int>, called where vec<string> expected.
function test_vec_return_type_mismatch(): void {
  ExampleDsl`{
    $f = () ==> vec[1, 2, 3];
    $g = (vec<ExampleString> $v) ==> $v;
    $g($f());
  }`;
}

// Lambda returns dict<string, int>, called where dict<string, string> expected.
function test_dict_return_type_mismatch(): void {
  ExampleDsl`{
    $f = () ==> dict["a" => 1];
    $g = (dict<ExampleString, ExampleString> $d) ==> $d;
    $g($f());
  }`;
}

// Inferred nested vec<int> doesn't match param vec<string>.
function test_nested_vec_inferred_wrong_param(): void {
  ExampleDsl`{
    $f = (vec<vec<ExampleString>> $v) ==> $v;
    $v = vec[vec[1, 2]];
    $f($v);
  }`;
}
