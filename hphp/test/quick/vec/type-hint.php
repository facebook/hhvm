<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {}

function takes_vec1(@vec $v): vec {
  return $v;
}
function takes_vec2(@vec<Foo> $v): vec<Foo> {
  return $v;
}
function takes_vec3(@?vec $v): ?vec {
  return $v;
}
function takes_container1(@Container<Foo> $c): Container<Foo> {
  return $c;
}
function takes_container2(@?Container<Foo> $c): ?Container<Foo> {
  return $c;
}
function takes_keyed_container(@KeyedContainer $c): KeyedContainer {
  return $c;
}
function takes_traversable(@Traversable<Foo> $t): Traversable<Foo> {
  return $t;
}
function takes_keyed_traversable(@KeyedTraversable $t): KeyedTraversable {
  return $t;
}
function takes_dict(@dict $d): dict {
  return $d;
}
function takes_keyset(@keyset $ks): keyset {
  return $ks;
}
function takes_bool(@bool $b): bool {
  return $b;
}
function takes_string(@string $s): string {
  return $s;
}
function takes_array1(@array $a): array {
  return $a;
}
function takes_array2(@?array $a): ?array {
  return $a;
}
function takes_xhp_child(@XHPChild $x): XHPChild {
  return $x;
}
function takes_foo(@Foo $f): Foo {
  return $f;
}
function takes_vector_container(@Vector $v): Vector {
  return $v;
}
function takes_map_container(@Map $m): Map {
  return $m;
}
function takes_null(@null $x): null {
  return $x;
}
function takes_nonnull(@nonnull $x): nonnull {
  return $x;
}
function takes_mixed(@mixed $m): mixed {
  return $m;
}
function takes_vec_or_dict1(@vec_or_dict $x): vec_or_dict {
  return $x;
}
function takes_vec_or_dict2(@?vec_or_dict $x): ?vec_or_dict {
  return $x;
}
function takes_vec_or_dict3(@vec_or_dict<int> $x): vec_or_dict<int> {
  return $x;
}

function test_all_hints($x) {
  echo "====================================================\n";
  var_dump($x);
  takes_vec1($x)
    |> takes_vec2($$)
    |> takes_vec3($$)
    |> takes_container1($$)
    |> takes_container2($$)
    |> takes_keyed_container($$)
    |> takes_traversable($$)
    |> takes_keyed_traversable($$)
    |> takes_dict($$)
    |> takes_keyset($$)
    |> takes_bool($$)
    |> takes_string($$)
    |> takes_array1($$)
    |> takes_array2($$)
    |> takes_xhp_child($$)
    |> takes_foo($$)
    |> takes_vector_container($$)
    |> takes_map_container($$)
    |> takes_null($$)
    |> takes_nonnull($$)
    |> takes_mixed($$)
    |> takes_vec_or_dict1($$)
    |> takes_vec_or_dict2($$)
    |> takes_vec_or_dict3($$)
    |> var_dump($$);
}

function test_vec_hint($x) {
  echo "====================================================\n";
  var_dump($x);
  takes_vec1($x)
    |> takes_vec2($$)
    |> takes_vec3($$)
    |> takes_vec_or_dict1($$)
    |> takes_vec_or_dict2($$)
    |> takes_vec_or_dict3($$)
    |> var_dump($$);
}
<<__EntryPoint>> function main(): void {
set_error_handler(
  (int $errno,
   string $errstr,
   string $errfile,
   int $errline,
   array $errcontext
  ) ==> {
    echo "ERROR: "; var_dump($errstr);
    return true;
  }
);

test_all_hints(vec[]);
test_all_hints(vec[1, 2, 3]);

test_vec_hint(null);
test_vec_hint(false);
test_vec_hint(717);
test_vec_hint(1.234);
test_vec_hint("string");
test_vec_hint(new stdclass);
test_vec_hint(varray[]);
test_vec_hint(varray[1, 2, 3]);
test_vec_hint(dict[]);
test_vec_hint(dict['a' => 1]);
test_vec_hint(keyset[]);
test_vec_hint(keyset['a', 1]);
test_vec_hint(Vector{1, 2, 3});
test_vec_hint(Map{1 => 'a', 2 => 'b'});
}
