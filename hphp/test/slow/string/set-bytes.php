<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

<<__NEVER_INLINE>>
function get_key() :mixed{
  return 'a';
}

function test_string() :mixed{
  echo ">>> Testing strings <<<\n";
  $s = "This string is 34 characters long.";
  $s2 = $s;

  var_dump($s);
  HH\set_bytes_string($s, 5, 'puppy dog', 6);
  var_dump($s);
  HH\set_bytes_rev_string($s, 5, ' yttik tac', 6);
  var_dump($s);
  HH\set_bytes_string($s, 0, $s);
  var_dump($s);
  HH\set_bytes_rev_string($s, 0, $s, 4);
  var_dump($s);
  HH\set_bytes_string($s, 0, "THIS STRING IS 34 CHARACTERS LONG!");
  var_dump($s);
  HH\set_bytes_rev_string($s, 5, "abcdefghijklmnopqrstuvwxyz123");
  var_dump($s);

  $a = dict['a' => 'this is inside a dict'];
  var_dump($a);
  HH\set_bytes_string($a[get_key()], 0, 'T');
  var_dump($a);

  var_dump($s2);
  echo ">>> End of strings <<<\n";
}

function test_primitive() :mixed{
  echo ">>> Testing primitive types <<<\n";
  $s = "The quick brown fox jumps over the lazy dog.";

  var_dump($s);
  HH\set_bytes_int8($s, 43, 33);
  var_dump($s);
  HH\set_bytes_int16($s, 4, 0x776b);
  var_dump($s);
  HH\set_bytes_int32($s, 40, 0);
  var_export($s);
  echo "\n";
  HH\set_bytes_int64($s, 36, 0x7461632065677261);
  var_dump($s);

  HH\set_bytes_bool($s, 3, false);
  var_export($s);
  echo "\n";

  HH\set_bytes_float32($s, 3, 128.25);
  var_dump(unpack('f', substr($s, 3))[1]);
  HH\set_bytes_float64($s, 12, 256.125);
  var_dump(unpack('d', substr($s, 12))[1]);

  echo ">>> End of primitive types <<<\n";
}

function test_vec() :mixed{
  echo ">>> Testing vecs <<<\n";
  $blank = str_repeat('-', 32);
  $s = $blank;

  $bools = vec[false, false, true, true];
  HH\set_bytes_bool_vec($s, 12, $bools);
  var_dump(unpack('c*', substr($s, 12, 6)));
  HH\set_bytes_rev_bool_vec($s, 12, $bools);
  var_dump(unpack('c*', substr($s, 12, 6)));

  $s = $blank;
  $h = vec[0x68, 0x65, 0x6c, 0x6c, 0x6f];
  var_dump($s);
  HH\set_bytes_int8_vec($s, 0, $h);
  var_dump($s);
  HH\set_bytes_rev_int8_vec($s, 0, $h);
  var_dump($s);

  $s = $blank;
  $p = vec[0x2121, 0x3f3f];
  HH\set_bytes_int16_vec($s, 12, $p, 1);
  var_dump($s);
  HH\set_bytes_int16_vec($s, 14, $p);
  var_dump($s);

  $b = vec[0x68616c62, 0x68616c62, 0x68656c62];
  HH\set_bytes_int32_vec($s, 4, $b);
  var_dump($s);
  HH\set_bytes_rev_int32_vec($s, 4, $b);
  var_dump($s);

  $s = $blank;
  $m = vec[7863412928448521555, 2315243178136138597];
  HH\set_bytes_int64_vec($s, 0, $m);
  var_dump($s);

  $s = $blank;
  $f = vec[1.0, 2.0, 3.0, 4.0];
  HH\set_bytes_float32_vec($s, 0, $f, 2);
  var_dump(unpack('f', substr($s, 0, 4))[1]);
  var_dump(unpack('f', substr($s, 4, 4))[1]);

  HH\set_bytes_float64_vec($s, 8, $f, 2);
  var_dump(unpack('d', substr($s, 8, 8))[1]);
  var_dump(unpack('d', substr($s, 16, 8))[1]);

  echo ">>> End of vecs <<<\n";
}

function try_func($f) :mixed{
  try {
    $f();
    echo "Function did not throw\n";
  } catch (Exception $e) {
    printf("Caught %s: %s\n", get_class($e), $e->getMessage());
  }
}

function failures() :mixed{
  echo ">>> Testing failure cases <<<\n";
  $s = "This string is 34 characters long.";

  // Reading too many bytes from string
  try_func(() ==> { HH\set_bytes_string($s, 0, 'str', 4); });
  // Size != 1 for string
  try_func(() ==> { HH\set_bytes_int16($s, 0, 'str'); });

  $bad_vec = vec[1, 2, 3.0];
  // Polymorphic vec
  try_func(() ==> { HH\set_bytes_int32_vec($s, 0, $bad_vec); });
  $vec = vec[1, 2, 3];
  // Reading too many elements from vec
  try_func(() ==> { HH\set_bytes_int32_vec($s, 0, $vec, 4); });

  // Type/size mismatch in vec
  try_func(() ==> { HH\set_bytes_bool_vec($s, 0, vec[1.0]); });
  try_func(() ==> { HH\set_bytes_int16_vec($s, 0, vec[1.0]); });
  try_func(() ==> { HH\set_bytes_int32_vec($s, 0, vec[false]); });
  try_func(() ==> { HH\set_bytes_int64_vec($s, 0, vec[true]); });

  $i = 12;
  // Bad base type
  try_func(() ==> { HH\set_bytes_int32($i, 0, $vec); });

  // Range check for different source types
  try_func(() ==> { HH\set_bytes_bool($s, 40, true); });
  try_func(() ==> { HH\set_bytes_int32($s, 32, 24); });
  try_func(() ==> { HH\set_bytes_float64($s, 27, 42.24); });
  try_func(() ==> { HH\set_bytes_string($s, 30, 'hello'); });
  try_func(() ==> { HH\set_bytes_int32_vec($s, 30, $vec); });

  // Bad type/size combinations for primitives
  try_func(() ==> { HH\set_bytes_int16($s, 0, true); });
  try_func(() ==> { HH\set_bytes_int8($s, 0, 42.24); });

  // rev on primitive types
  try_func(() ==> { HH\set_bytes_rev_bool($s, 0, false); });
  try_func(() ==> { HH\set_bytes_rev_int32($s, 0, 5); });
  try_func(() ==> { HH\set_bytes_rev_float64($s, 0, 5.0); });

  // ignore inout modifiers on first parameter
  $s = "This string is 34 characters long.";
  var_dump($s);
  HH\set_bytes_string(inout $s, 5, 'branch', 6);
  var_dump($s);
  HH\set_bytes_int8(inout $s, 33, 63);
  var_dump($s);

  echo ">>> End of failure cases <<<\n";
}

<<__EntryPoint>> function main(): void {
  test_string();
  test_primitive();
  test_vec();
  failures();
}
