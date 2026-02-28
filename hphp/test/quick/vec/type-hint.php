<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {}

function takes_vec1(<<__Soft>> vec $v): vec {
  return $v;
}
function takes_vec2(<<__Soft>> vec<Foo> $v): vec<Foo> {
  return $v;
}
function takes_vec3(<<__Soft>> ?vec $v): ?vec {
  return $v;
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
function takes_dict(<<__Soft>> dict $d): dict {
  return $d;
}
function takes_keyset(<<__Soft>> keyset $ks): keyset {
  return $ks;
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
function takes_vec_or_dict1(<<__Soft>> vec_or_dict $x): vec_or_dict {
  return $x;
}
function takes_vec_or_dict2(<<__Soft>> ?vec_or_dict $x): ?vec_or_dict {
  return $x;
}
function takes_vec_or_dict3(<<__Soft>> vec_or_dict<int> $x): vec_or_dict<int> {
  return $x;
}

function test_all_hints($x) :mixed{
  echo "====================================================\n";
  var_dump($x);
  $funs = vec[
    takes_vec1<>,
    takes_vec2<>,
    takes_vec3<>,
    takes_container1<>,
    takes_container2<>,
    takes_keyed_container<>,
    takes_traversable<>,
    takes_keyed_traversable<>,
    takes_dict<>,
    takes_keyset<>,
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
    takes_vec_or_dict1<>,
    takes_vec_or_dict2<>,
    takes_vec_or_dict3<>,
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

function test_vec_hint($x) :mixed{
  echo "====================================================\n";
  var_dump($x);
  $funs = vec[
    takes_vec1<>,
    takes_vec2<>,
    takes_vec3<>,
    takes_vec_or_dict1<>,
    takes_vec_or_dict2<>,
    takes_vec_or_dict3<>,
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

test_all_hints(vec[]);
test_all_hints(vec[1, 2, 3]);

test_vec_hint(null);
test_vec_hint(false);
test_vec_hint(717);
test_vec_hint(1.234);
test_vec_hint("string");
test_vec_hint(new stdClass);
test_vec_hint(vec[]);
test_vec_hint(vec[1, 2, 3]);
test_vec_hint(dict[]);
test_vec_hint(dict['a' => 1]);
test_vec_hint(keyset[]);
test_vec_hint(keyset['a', 1]);
test_vec_hint(Vector{1, 2, 3});
test_vec_hint(Map{1 => 'a', 2 => 'b'});
}
