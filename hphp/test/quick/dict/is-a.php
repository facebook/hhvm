<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Foo {}

function test_is_a($a, $interfaces) {
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

  echo "\tis_array: ";
  var_dump(is_array($a));

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

  echo "instanceof Traversable: ";
  var_dump($a instanceof Traversable);

  echo "instanceof KeyedTraversable: ";
  var_dump($a instanceof KeyedTraversable);

  echo "instanceof Container: ";
  var_dump($a instanceof Container);

  echo "instanceof KeyedContainer: ";
  var_dump($a instanceof KeyedContainer);

  echo "instanceof Indexish: ";
  var_dump($a instanceof Indexish);

  echo "instanceof XHPChild: ";
  var_dump($a instanceof XHPChild);

  echo "instanceof ArrayAccess: ";
  var_dump($a instanceof ArrayAccess);

  echo "instanceof Vector: ";
  var_dump($a instanceof Vector);

  echo "instanceof Map: ";
  var_dump($a instanceof Map);

  echo "instanceof Foo: ";
  var_dump($a instanceof Foo);

  foreach ($interfaces as $i) {
    echo "instanceof (string) " . $i . ": ";
    var_dump($a instanceof $i);
  }
}

function test_is_dict($val) {
  echo "====================================================\n";
  echo "Testing for is_dict: ";
  var_dump($val);
  if (is_dict($val)) {
    echo "YES\n";
  } else {
    echo "NO\n";
  }
}

function main() {
  $interfaces = [
    "HH\\Traversable",
    "HH\\KeyedTraversable",
    "HH\\Container",
    "HH\\KeyedContainer",
    "Indexish",
    "XHPChild",
    "ArrayAccess",
    "Vector",
    "Map",
    "Foo",
  ];

  test_is_a(dict[], $interfaces);
  test_is_a(dict['a' => 100, 200 => 'b'], $interfaces);

  test_is_dict(null);
  test_is_dict(false);
  test_is_dict(7);
  test_is_dict(1.23);
  test_is_dict("abcd");
  test_is_dict(new stdclass);
  test_is_dict([1, 2, 3]);
  test_is_dict(Vector{'a', 'b', 'c'});
  test_is_dict(Map{100 => 'a', 'b' => 200});
  test_is_dict(Pair{123, 'abc'});

  $resource = imagecreate(1, 1);
  test_is_dict($resource);
  imagedestroy($resource);

}
main();
