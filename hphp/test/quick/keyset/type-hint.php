<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {}

function takes_keyset1(keyset $ks): keyset {
  return $ks;
}
function takes_keyset2(keyset<int> $ks): keyset<int> {
  return $ks;
}
function takes_keyset3(keyset<string> $ks): keyset<string> {
  return $ks;
}
function takes_keyset4(?keyset $ks): ?keyset {
  return $ks;
}
function takes_container1(Container<Foo> $c): Container<Foo> {
  return $c;
}
function takes_container2(?Container<Foo> $c): ?Container<Foo> {
  return $c;
}
function takes_keyed_container(KeyedContainer $c): KeyedContainer {
  return $c;
}
function takes_traversable(Traversable<Foo> $t): Traversable<Foo> {
  return $t;
}
function takes_keyed_traversable(KeyedTraversable $t): KeyedTraversable {
  return $t;
}
function takes_vec(vec $v): vec {
  return $v;
}
function takes_dict(dict $d): dict {
  return $d;
}
function takes_bool(bool $b): bool {
  return $b;
}
function takes_string(string $s): string {
  return $s;
}
function takes_array1(array $a): array {
  return $a;
}
function takes_array2(?array $a): ?array {
  return $a;
}
function takes_indexish(Indexish $x): Indexish {
  return $x;
}
function takes_xhp_child(XHPChild $x): XHPChild {
  return $x;
}
function takes_foo(Foo $f): Foo {
  return $f;
}
function takes_array_access(ArrayAccess<int, Foo> $a): ArrayAccess<int, Foo> {
  return $a;
}
function takes_vector_container(Vector $v): Vector {
  return $v;
}
function takes_map_container(Map $m): Map {
  return $m;
}
function takes_mixed(mixed $m): mixed {
  return $m;
}

function test_all_hints($x) {
  echo "====================================================\n";
  var_dump($x);
  takes_keyset1($x)
    |> takes_keyset2($$)
    |> takes_keyset3($$)
    |> takes_keyset4($$)
    |> takes_container1($$)
    |> takes_container2($$)
    |> takes_keyed_container($$)
    |> takes_traversable($$)
    |> takes_keyed_traversable($$)
    |> takes_vec($$)
    |> takes_dict($$)
    |> takes_bool($$)
    |> takes_string($$)
    |> takes_array1($$)
    |> takes_array2($$)
    |> takes_indexish($$)
    |> takes_xhp_child($$)
    |> takes_foo($$)
    |> takes_array_access($$)
    |> takes_vector_container($$)
    |> takes_map_container($$)
    |> takes_mixed($$)
    |> var_dump($$);
}

function test_keyset_hint($x) {
  echo "====================================================\n";
  var_dump($x);
  takes_keyset1($x)
    |> takes_keyset2($$)
    |> takes_keyset3($$)
    |> takes_keyset4($$)
    |> var_dump($$);
}

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

test_all_hints(keyset[]);
test_all_hints(keyset[1, 'a', 2, 'b']);

test_keyset_hint(null);
test_keyset_hint(false);
test_keyset_hint(717);
test_keyset_hint(1.234);
test_keyset_hint("string");
test_keyset_hint(new stdclass);
test_keyset_hint([]);
test_keyset_hint([1, 2, 3]);
test_keyset_hint(vec[]);
test_keyset_hint(vec[1, 'a', 2, 'b']);
test_keyset_hint(dict[]);
test_keyset_hint(dict['a' => 1]);
test_keyset_hint(Vector{1, 2, 3});
test_keyset_hint(Map{1 => 'a', 2 => 'b'});
