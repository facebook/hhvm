<?hh

<<file:__EnableUnstableFeatures('expression_trees', 'expression_tree_hack_arrays')>>

// Tests that HH infers collection element types correctly and flows them
// through assignments, returns, and function calls — all should pass.

// Inferred vec type flows through assignment into typed param.
function test_vec_inferred_to_typed_param(): void {
  ExampleDsl`{
    $f = (vec<ExampleInt> $v) ==> $v;
    $v = vec[1, 2, 3];
    $f($v);
  }`;
}

// Inferred dict type flows through assignment.
function test_dict_inferred_to_typed_param(): void {
  ExampleDsl`{
    $f = (dict<ExampleString, ExampleInt> $d) ==> $d;
    $d = dict["a" => 1, "b" => 2];
    $f($d);
  }`;
}

// Inferred keyset type flows through assignment.
function test_keyset_inferred_to_typed_param(): void {
  ExampleDsl`{
    $f = (keyset<ExampleInt> $k) ==> $k;
    $k = keyset[1, 2, 3];
    $f($k);
  }`;
}

// Lambda returning a vec — return type should be inferred.
function test_vec_return_type_inference(): void {
  ExampleDsl`{
    $f = () ==> vec[1, 2, 3];
    $g = (vec<ExampleInt> $v) ==> $v;
    $g($f());
  }`;
}

// Lambda returning a dict.
function test_dict_return_type_inference(): void {
  ExampleDsl`{
    $f = () ==> dict["a" => 1];
    $g = (dict<ExampleString, ExampleInt> $d) ==> $d;
    $g($f());
  }`;
}

// Lambda returning a keyset.
function test_keyset_return_type_inference(): void {
  ExampleDsl`{
    $f = () ==> keyset[1, 2];
    $g = (keyset<ExampleInt> $k) ==> $k;
    $g($f());
  }`;
}

// Multiple assignments — type should persist.
function test_vec_type_persists_through_reassignment(): void {
  ExampleDsl`{
    $v = vec[1, 2];
    $w = $v;
    $f = (vec<ExampleInt> $x) ==> $x;
    $f($w);
  }`;
}

// Collection built up and passed through.
function test_dict_type_persists_through_reassignment(): void {
  ExampleDsl`{
    $d = dict["a" => 1];
    $e = $d;
    $f = (dict<ExampleString, ExampleInt> $x) ==> $x;
    $f($e);
  }`;
}
