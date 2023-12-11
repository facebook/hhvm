<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {}

function takes_dict1(<<__Soft>> dict $foo): dict {
  return $foo;
}
function takes_dict2(<<__Soft>> dict<Foo> $bar): dict<Foo> {
  return $bar;
}
function takes_dict3(<<__Soft>> dict<Foo,> $baz): dict<Foo,> {
  return $baz;
}
function takes_dict4(<<__Soft>> dict<Foo, Bar> $biz): dict<Foo, Bar> {
  return $biz;
}
function takes_dict5(<<__Soft>> ?dict $d): ?dict {
  return $d;
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
function takes_keyset(<<__Soft>> keyset $ks): keyset {
  return $ks;
}
function takes_array1(<<__Soft>> varray $arr): varray {
  return $arr;
}
function takes_array2(<<__Soft>> ?varray $arr): ?varray {
  return $arr;
}
function takes_bool(<<__Soft>> bool $b): bool {
  return $b;
}
function takes_string(<<__Soft>> string $s): string {
  return $s;
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
    takes_dict1<>,
    takes_dict2<>,
    takes_dict3<>,
    takes_dict4<>,
    takes_dict5<>,
    takes_container1<>,
    takes_container2<>,
    takes_keyed_container<>,
    takes_traversable<>,
    takes_keyed_traversable<>,
    takes_vec<>,
    takes_keyset<>,
    takes_array1<>,
    takes_array2<>,
    takes_bool<>,
    takes_string<>,
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

function test_dict_hint($x) :mixed{
  echo "====================================================\n";
  var_dump($x);
  $funs = vec[
    takes_dict1<>,
    takes_dict2<>,
    takes_dict3<>,
    takes_dict4<>,
    takes_dict5<>,
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

test_all_hints(dict[]);
test_all_hints(dict[1 => 'a', 'b' => 2]);

test_dict_hint(null);
test_dict_hint(false);
test_dict_hint(717);
test_dict_hint(1.234);
test_dict_hint("string");
test_dict_hint(new stdClass);
test_dict_hint(vec[]);
test_dict_hint(vec[1, 2, 3]);
test_dict_hint(vec[]);
test_dict_hint(vec[1, 2, 3]);
test_dict_hint(keyset[]);
test_dict_hint(keyset['a', 1]);
test_dict_hint(Vector{1, 2, 3});
test_dict_hint(Map{1 => 'a', 2 => 'b'});
}
