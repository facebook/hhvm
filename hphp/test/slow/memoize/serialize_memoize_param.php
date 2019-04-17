<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls1 implements HH\IMemoizeParam {
  public function getInstanceKey() { return "Cls1 memo key"; }
}

class Cls2 implements HH\IMemoizeParam {
  public function getInstanceKey() { throw new Exception("clowntown"); }
}

class Cls3 {}

function escaped_print_serialized($x) {
  $x = HH\serialize_memoize_param($x);
  if (!is_string($x)) {
    var_dump($x);
    return;
  }
  var_dump(quoted_printable_encode($x));
}

function test_group($name, $inputs) {
  echo "==== $name ====\n";
  foreach ($inputs as $input) {
    escaped_print_serialized($input);
  }
}

function test_exception($name, $input) {
  echo "==== $name ====\n";
  try {
    escaped_print_serialized($input);
  } catch (InvalidArgumentException $iae) {
    echo "InvalidArgumentException: ";
    var_dump($iae->getMessage());
  } catch (Exception $e) {
    echo "Exception of class '" . get_class($e) . "' with message: ";
    var_dump($e->getMessage());
  }
}

function test() {
  test_group(
    "ints and most strings pass through unchanged",
    [
      0, 123, -123,
      "", "hello world", "123", "~abcdef", "a\xf0\xff",
    ]
  );

  test_group(
    "strings where the first byte >= 0xf0",
    [
      "\xf0", "\xf1", "\xff",
      "\xf0normal", "\xf1stuff", "\xfffollows",
    ]
  );

  test_group(
    "strings (in an array)",
    [
      [""],
      ["hello world"],
      ["123"],
    ]
  );

  test_group(
    "1 byte int (in an array)",
    [ [0], [1], [-1], [2**7 - 1], [-(2**7)] ]
  );
  test_group(
    "2 byte int (in an array)",
    [ [2**7], [-(2**7 + 1)], [2**15 - 1], [-(2**15)] ]
  );
  test_group(
    "4 byte int (in an array)",
    [ [2**15], [-(2**15 + 1)], [2**31 - 1], [-(2**31)] ]
  );
  test_group(
    "8 byte int (in an array)",
    [
      [2**31],
      [-(2**31 + 1)],
      [(2**62 - 1) * 2 + 1], // = 2**63 - 1, but 2**63 gets turned into a double
      [-(2**62) * 2], // = -(2**63), but 2**63 gets turned into a double
    ]
  );

  test_group(
    "other basic types",
    [
      null,
      true, false,
      1.234,
    ]
  );
  test_group(
    "other basic types (in an array)",
    [
      [null],
      [true], [false],
      [1.234],
    ]
  );

  $emptyFormerPackedArray = [1];
  unset($emptyFormerPackedArray[0]);
  $emptyFormerMixedArray = [1 => 2];
  unset($emptyFormerMixedArray[1]);
  test_group(
    "empty containers",
    [
      [],
      $emptyFormerPackedArray,
      $emptyFormerMixedArray,
      Vector {},
      Map {},
      Set {},
      vec[],
      dict[],
      keyset[],
    ]
  );
  test_group(
    "containers with something in them",
    [
      [1, 2],
      [1 => 1, 2 => 2],
      Vector {1, 2},
      Map {1 => 1, 2 => 2},
      Set {1, 2},
      Pair {1, 2},
      vec[1, 2],
      dict[1 => 1, 2 => 2],
      keyset[1, 2],
    ]
  );
  test_group(
    "insertion order always matters",
    [
      [2, 1],
      [2 => 2, 1 => 1],
      Vector {2, 1},
      Map {2 => 2, 1 => 1},
      Set {2, 1},
      Pair {2, 1},
      vec[2, 1],
      dict[2 => 2, 1 => 1],
      keyset[2, 1],
    ]
  );
  test_group(
    "containers inside containers",
    [
      // the serializations for these 2 differ by the position of 1 STOP byte
      [[1 => 1]],
      [[], 1],
    ]
  );

  test_group("instance of class that implements IMemoizeParam", [new Cls1()]);

  test_exception(
    "instance of class where IMemoizeParam throws",
    new Cls2()
  );

  test_exception(
    "instance of class that doesn't implement IMemoizeParam",
    new Cls3()
  );

  test_exception("resource", imagecreate(1, 1));

  $wrapped = [0];
  for ($i = 0; $i < 128; $i++) {
    $wrapped = Vector{[$wrapped]};
  }
  test_exception("recursion depth", $wrapped);
}


<<__EntryPoint>>
function main_serialize_memoize_param() {
test();
}
