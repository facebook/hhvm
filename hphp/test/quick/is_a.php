<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {
  public function bar(): void {}
}

function reified_fn<reify T>(T $x): void {}
function not_reified_fn<T>(T $x): void {}

function test_is_a($a) :mixed{
  $interfaces = vec[
    "HH\\Traversable",
    "HH\\KeyedTraversable",
    "HH\\Container",
    "HH\\KeyedContainer",
    "XHPChild",
    "Vector",
    "Map",
    "Foo",
  ];

  echo "====================================================\n";
  echo "Testing: ";
  var_dump($a);

  echo "\tgettype: ";
  var_dump(gettype($a));

  echo "\tis_null: ";
  var_dump(is_null($a));

  echo "\tis_bool: ";
  var_dump(is_bool($a));

  echo "\tis_int: ";
  var_dump(is_int($a));

  echo "\tis_float: ";
  var_dump(is_float($a));

  echo "\tis_numeric: ";
  var_dump(is_numeric($a));

  echo "\tis_string: ";
  var_dump(is_string($a));

  echo "\tis_scalar: ";
  var_dump(is_scalar($a));

  echo "\tis_php_array: ";
  var_dump(HH\is_php_array($a));

  echo "\tis_vec: ";
  var_dump(is_vec($a));

  echo "\tis_dict: ";
  var_dump(is_dict($a));

  echo "\tis_keyset: ";
  var_dump(is_keyset($a));

  echo "\tis_object: ";
  var_dump(is_object($a));

  echo "\tis_resource: ";
  var_dump(is_resource($a));

  echo "is Traversable: ";
  var_dump($a is Traversable);

  echo "is KeyedTraversable: ";
  var_dump($a is KeyedTraversable);

  echo "is Container: ";
  var_dump($a is Container);

  echo "is KeyedContainer: ";
  var_dump($a is KeyedContainer);

  echo "is XHPChild: ";
  var_dump($a is XHPChild);

  echo "is Vector: ";
  var_dump($a is Vector);

  echo "is Map: ";
  var_dump($a is Map);

  echo "is Foo: ";
  var_dump($a is Foo);

  foreach ($interfaces as $i) {
    echo "is (string) " . $i . ": ";
    var_dump(is_a($a, $i));
  }
}


<<__EntryPoint>>
function main(): void {
  test_is_a(null);
  test_is_a(false);
  test_is_a(7);
  test_is_a(1.23);
  test_is_a("abcd");
  test_is_a(new stdClass);
  test_is_a(nameof Foo);
  test_is_a(Foo::class);
  test_is_a(vec[1, 2, 3]);
  test_is_a(keyset[1, 2, 3]);
  test_is_a(dict[1 => 2, 3 => 4]);
  test_is_a(Vector{'a', 'b', 'c'});
  test_is_a(Map{100 => 'a', 'b' => 200});
  test_is_a(Pair{123, 'abc'});
  test_is_a(main<>);
  test_is_a(reified_fn<int>);
  test_is_a(not_reified_fn<int>);
  test_is_a(meth_caller(Foo::class, 'bar'));
  test_is_a(#Foo);

  $resource = imagecreate(1, 1);
  test_is_a($resource);
  imagedestroy($resource);
}
