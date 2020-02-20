<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {}

function takes_dict1(@dict $foo): dict {
  return $foo;
}
function takes_dict2(@dict<Foo> $bar): dict<Foo> {
  return $bar;
}
function takes_dict3(@dict<Foo,> $baz): dict<Foo,> {
  return $baz;
}
function takes_dict4(@dict<Foo, Bar> $biz): dict<Foo, Bar> {
  return $biz;
}
function takes_dict5(@?dict $d): ?dict {
  return $d;
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
function takes_vec(@vec $v): vec {
  return $v;
}
function takes_keyset(@keyset $ks): keyset {
  return $ks;
}
function takes_array1(@array $arr): array {
  return $arr;
}
function takes_array2(@?array $arr): ?array {
  return $arr;
}
function takes_bool(@bool $b): bool {
  return $b;
}
function takes_string(@string $s): string {
  return $s;
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
  takes_dict1($x)
    |> takes_dict2($$)
    |> takes_dict3($$)
    |> takes_dict4($$)
    |> takes_dict5($$)
    |> takes_container1($$)
    |> takes_container2($$)
    |> takes_keyed_container($$)
    |> takes_traversable($$)
    |> takes_keyed_traversable($$)
    |> takes_vec($$)
    |> takes_keyset($$)
    |> takes_array1($$)
    |> takes_array2($$)
    |> takes_bool($$)
    |> takes_string($$)
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

function test_dict_hint($x) {
  echo "====================================================\n";
  var_dump($x);
  takes_dict1($x)
    |> takes_dict2($$)
    |> takes_dict3($$)
    |> takes_dict4($$)
    |> takes_dict5($$)
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

test_all_hints(dict[]);
test_all_hints(dict[1 => 'a', 'b' => 2]);

test_dict_hint(null);
test_dict_hint(false);
test_dict_hint(717);
test_dict_hint(1.234);
test_dict_hint("string");
test_dict_hint(new stdclass);
test_dict_hint(varray[]);
test_dict_hint(varray[1, 2, 3]);
test_dict_hint(vec[]);
test_dict_hint(vec[1, 2, 3]);
test_dict_hint(keyset[]);
test_dict_hint(keyset['a', 1]);
test_dict_hint(Vector{1, 2, 3});
test_dict_hint(Map{1 => 'a', 2 => 'b'});
}
