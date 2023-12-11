<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {}

function takes_keyset1(<<__Soft>> keyset $ks): keyset {
  return $ks;
}
function takes_keyset2(<<__Soft>> keyset<int> $ks): keyset<int> {
  return $ks;
}
function takes_keyset3(<<__Soft>> keyset<string> $ks): keyset<string> {
  return $ks;
}
function takes_keyset4(<<__Soft>> ?keyset $ks): ?keyset {
  return $ks;
}
function takes_container1(<<__Soft>> Container<Foo> $c): Container<Foo> {
  return $c;
}
function takes_container2(<<__Soft>> ?Container<Foo> $c): ?Container<Foo> {
  return $c;
}
function takes_keyed_container(<<__Soft>> KeyedContainer $c): KeyedContainer {
  return $c;
}
function takes_traversable(<<__Soft>> Traversable<Foo> $t): Traversable<Foo> {
  return $t;
}
function takes_keyed_traversable(<<__Soft>> KeyedTraversable $t): KeyedTraversable {
  return $t;
}
function takes_vec(<<__Soft>> vec $v): vec {
  return $v;
}
function takes_dict(<<__Soft>> dict $d): dict {
  return $d;
}
function takes_bool(<<__Soft>> bool $b): bool {
  return $b;
}
function takes_string(<<__Soft>> string $s): string {
  return $s;
}
function takes_array1(<<__Soft>> varray $a): varray {
  return $a;
}
function takes_array2(<<__Soft>> ?varray $a): ?varray {
  return $a;
}
function takes_xhp_child(<<__Soft>> XHPChild $x): XHPChild {
  return $x;
}
function takes_foo(<<__Soft>> Foo $f): Foo {
  return $f;
}
function takes_vector_container(<<__Soft>> Vector $v): Vector {
  return $v;
}
function takes_map_container(<<__Soft>> Map $m): Map {
  return $m;
}
function takes_null(<<__Soft>> null $x): null {
  return $x;
}
function takes_nonnull(<<__Soft>> nonnull $x): nonnull {
  return $x;
}
function takes_mixed(<<__Soft>> mixed $m): mixed {
  return $m;
}

function test_all_hints($x) :mixed{
  echo "====================================================\n";
  var_dump($x);
  $funs = vec[
    takes_keyset1<>,
    takes_keyset2<>,
    takes_keyset3<>,
    takes_keyset4<>,
    takes_container1<>,
    takes_container2<>,
    takes_keyed_container<>,
    takes_traversable<>,
    takes_keyed_traversable<>,
    takes_vec<>,
    takes_dict<>,
    takes_bool<>,
    takes_string<>,
    takes_array1<>,
    takes_array2<>,
    takes_xhp_child<>,
    takes_foo<>,
    takes_vector_container<>,
    takes_map_container<>,
    takes_null<>,
    takes_nonnull<>,
    takes_mixed<>,
   ];

  foreach($funs as $fun) {
    try {
      $x = $fun($x);
    } catch (Exception $e) {
      echo "ERROR: "; var_dump($e->getMessage());
    }
  }
  var_dump($x);
}

function test_keyset_hint($x) :mixed{
  echo "====================================================\n";
  var_dump($x);
  $funs = vec[
    takes_keyset1<>,
    takes_keyset2<>,
    takes_keyset3<>,
    takes_keyset4<>,
  ];
  foreach($funs as $fun) {
    try {
      $x = $fun($x);
    } catch (Exception $e) {
      echo "ERROR: "; var_dump($e->getMessage());
    }
  }
  var_dump($x);
}

<<__EntryPoint>> function main(): void {
set_error_handler(
  (int $errno,
   string $errstr,
   string $errfile,
   int $errline,
   darray $errcontext
  ) ==> {
    if (strpos($errstr, 'returned from')) {
      throw new Exception($errstr);
    }
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
test_keyset_hint(new stdClass);
test_keyset_hint(vec[]);
test_keyset_hint(vec[1, 2, 3]);
test_keyset_hint(vec[]);
test_keyset_hint(vec[1, 'a', 2, 'b']);
test_keyset_hint(dict[]);
test_keyset_hint(dict['a' => 1]);
test_keyset_hint(Vector{1, 2, 3});
test_keyset_hint(Map{1 => 'a', 2 => 'b'});
}
