<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class Cls1 implements HH\IMemoizeParam {
  public function getInstanceKey() :mixed{ return "Cls1 memo key"; }
}

class Cls2 implements HH\IMemoizeParam {
  public function getInstanceKey() :mixed{ throw new Exception("clowntown"); }
}

class Cls3 {}

function escaped_print_serialized($x) :mixed{
  $x = HH\serialize_memoize_param($x);
  if (!is_string($x)) {
    var_dump($x);
    return;
  }
  var_dump(quoted_printable_encode($x));
}

function test_group($name, $inputs) :mixed{
  echo "==== $name ====\n";
  foreach ($inputs as $input) {
    escaped_print_serialized($input);
  }
}

function test_exception($name, $input) :mixed{
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

function test() :mixed{
  test_group(
    "ints and most strings pass through unchanged",
    varray[
      0, 123, -123,
      "", "hello world", "123", "~abcdef", "a\xf0\xff",
    ]
  );

  test_group(
    "strings where the first byte >= 0xf0",
    varray[
      "\xf0", "\xf1", "\xff",
      "\xf0normal", "\xf1stuff", "\xfffollows",
    ]
  );

  test_group(
    "strings (in an array)",
    varray[
      varray[""],
      varray["hello world"],
      varray["123"],
    ]
  );

  test_group(
    "1 byte int (in an array)",
    varray[ varray[0], varray[1], varray[-1], varray[2**7 - 1], varray[-(2**7)] ]
  );
  test_group(
    "2 byte int (in an array)",
    varray[ varray[2**7], varray[-(2**7 + 1)], varray[2**15 - 1], varray[-(2**15)] ]
  );
  test_group(
    "4 byte int (in an array)",
    varray[ varray[2**15], varray[-(2**15 + 1)], varray[2**31 - 1], varray[-(2**31)] ]
  );
  test_group(
    "8 byte int (in an array)",
    varray[
      varray[2**31],
      varray[-(2**31 + 1)],
      varray[(2**62 - 1) * 2 + 1], // = 2**63 - 1, but 2**63 gets turned into a double
      varray[-(2**62) * 2], // = -(2**63), but 2**63 gets turned into a double
    ]
  );

  test_group(
    "other basic types",
    varray[
      null,
      true, false,
      1.234,
    ]
  );
  test_group(
    "other basic types (in an array)",
    varray[
      varray[null],
      varray[true], varray[false],
      varray[1.234],
    ]
  );

  $emptyFormerPackedArray = varray[1];
  unset($emptyFormerPackedArray[0]);
  $emptyFormerMixedArray = darray[1 => 2];
  unset($emptyFormerMixedArray[1]);
  test_group(
    "empty containers",
    varray[
      varray[],
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
    varray[
      varray[1, 2],
      darray[1 => 1, 2 => 2],
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
    varray[
      varray[2, 1],
      darray[2 => 2, 1 => 1],
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
    varray[
      // the serializations for these 2 differ by the position of 1 STOP byte
      varray[darray[1 => 1]],
      varray[darray[], 1],
    ]
  );

  test_group("instance of class that implements IMemoizeParam", varray[new Cls1()]);

  test_exception(
    "instance of class where IMemoizeParam throws",
    new Cls2()
  );

  test_exception(
    "instance of class that doesn't implement IMemoizeParam",
    new Cls3()
  );

  test_exception("resource", imagecreate(1, 1));

  $wrapped = varray[0];
  for ($i = 0; $i < 128; $i++) {
    $wrapped = Vector{varray[$wrapped]};
  }
  test_exception("recursion depth", $wrapped);
}


<<__EntryPoint>>
function main_serialize_memoize_param() :mixed{
test();
}
