<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function test_append($orig) :mixed{
  echo "Testing append....\n";
  $v = $orig;
  $v[] = 0;
  $v[] = 'a';
  $v[] = 1.23;
  $v[] = false;
  $v[] = null;
  $v[] = new stdClass();
  $v[] = vec[300, 200, 100];
  $v[] = dict[];
  $v[] = dict['a' => 1, 'b' => 2, 'c' => 3];
  $v[] = vec[1, 2, 3];
  $v[] = keyset['a', 'b', 'c'];
  var_dump($orig);
  var_dump($v);
}

function do_set($dict, $key, $val) :mixed{
  try {
    $dict[$key] = $val;
    echo "Set to $val succeeded\n";
  } catch (Exception $e) {
    echo "Set to $val failed: \"", $e->getMessage(), "\"\n";
  }
  return $dict;
}

function test_set($orig) :mixed{
  echo "Testing set....\n";
  $d = do_set($orig, 0, "0 key value")
    |> do_set($$, 3, "3 key value")
    |> do_set($$, 999999999, "999999999 key value")
    |> do_set($$, -1, "-1 key value")
    |> do_set($$, "key1", "\"key1\" key value")
    |> do_set($$, "0", "\"0\" key value")
    |> do_set($$, "3", "\"3\" key value")
    |> do_set($$, "", "\"\" key value")
    |> do_set($$, false, "bool key value")
    |> do_set($$, null, "null key value")
    |> do_set($$, 1.23, "double key value")
    |> do_set($$, new stdClass(), "object key value")
    |> do_set($$, vec[1, 2, 3], "array key value")
    |> do_set($$, vec[1, 2, 3], "vec key value")
    |> do_set($$, dict['a' => 1, 'b' => 2, 'c' => 3], "dict key value")
    |> do_set($$, keyset['a', 'b', 'c'], "keyset key value");
  var_dump($orig);
  var_dump($d);
}

function do_setop($dict, $key, $val) :mixed{
  try {
    $dict[$key] .= $val;
    echo "Set-Op to \"$val\" succeeded\n";
  } catch (Exception $e) {
    echo "Set-Op to \"$val\" failed: \"", $e->getMessage(), "\"\n";
  }
  return $dict;
}

function test_setop($orig) :mixed{
  echo "Test set-op....\n";
  $d = do_setop($orig, 0, " + 0 key value")
    |> do_setop($$, 3, " + 3 key value")
    |> do_setop($$, 999999999, " + 999999999 key value")
    |> do_setop($$, -1, " + -1 key value")
    |> do_setop($$, "key1", " + \"key1\" key value")
    |> do_setop($$, "0", " + \"0\" key value")
    |> do_setop($$, "3", " + \"3\" key value")
    |> do_setop($$, "", " + \"\" key value")
    |> do_setop($$, false, " + bool key value")
    |> do_setop($$, null, " + null key value")
    |> do_setop($$, 1.23, " + double key value")
    |> do_setop($$, new stdClass(), " + object key value")
    |> do_setop($$, vec[1, 2, 3], " + array key value")
    |> do_setop($$, vec[1, 2, 3], " + vec key value")
    |> do_setop($$, dict['a' => 1, 'b' => 2, 'c' => 3], " + dict key value")
    |> do_setop($$, keyset['a', 'b', 'c'], " + keyset key value");
  var_dump($orig);
  var_dump($d);
}

function do_new_setop($vec, $val, $s) :mixed{
  try {
    $vec[] .= $val;
    echo "New set-op with \"$s\" succeeded\n";
  } catch (Exception $e) {
    echo "New set-op with \"$s\" failed: \"", $e->getMessage(), "\"\n";
  }
  return $vec;
}

function test_new_setop($orig) :mixed{
  echo "Test new set-op....\n";
  $v = do_new_setop($orig, 0, " + 0 value")
    |> do_new_setop($$, 3, " + 3 value")
    |> do_new_setop($$, 999999999, " + 999999999 value")
    |> do_new_setop($$, -1, " + -1 value")
    |> do_new_setop($$, "a", " + \"a\" value")
    |> do_new_setop($$, "key1", " + \"key1\" value")
    |> do_new_setop($$, "0", " + \"0\" value")
    |> do_new_setop($$, "3", " + \"3\" value")
    |> do_new_setop($$, "", " + \"\" value")
    |> do_new_setop($$, false, " + bool value")
    |> do_new_setop($$, null, " + null value")
    |> do_new_setop($$, 1.23, " + double value")
    |> do_new_setop($$, new stdClass(), " + object value")
    |> do_new_setop($$, vec[1, 2, 3], " + array value")
    |> do_new_setop($$, vec[1, 2, 3], " + vec value")
    |> do_new_setop($$, dict['a' => 1, 'b' => 2, 'c' => 3], " + dict value")
    |> do_new_setop($$, keyset['a', 'b', 'c'], " + keyset value");
  var_dump($orig);
  var_dump($v);
}

function do_unset($dict, $key, $str) :mixed{
  try {
    unset($dict[$key]);
    echo "Unset of $str succeeded\n";
  } catch (Exception $e) {
    echo "Unset of $str failed: \"", $e->getMessage(), "\"\n";
  }
  return $dict;
}

function test_unset($orig) :mixed{
  echo "Test unset....\n";
  $dict1 = do_unset($orig, 0, "0 key value");
  var_dump($orig);
  var_dump($dict1);

  $dict2 = do_unset($orig, 3, "3 key value");
  var_dump($orig);
  var_dump($dict2);

  $dict3 = do_unset($orig, 999999999, "999999999 key value")
    |> do_unset($$, -1, "-1 key value")
    |> do_unset($$, "key1", "\"key1\" key value")
    |> do_unset($$, "0", "\"0\" key value")
    |> do_unset($$, "3", "\"3\" key value")
    |> do_unset($$, "", "\"\" key value")
    |> do_unset($$, false, "false key value")
    |> do_unset($$, null, "null key value")
    |> do_unset($$, 1.23, "double key value")
    |> do_unset($$, new stdClass(), "object key value")
    |> do_unset($$, vec[1, 2, 3], "array key value")
    |> do_unset($$, vec[1, 2, 3], "vec key value")
    |> do_unset($$, dict['a' => 1, 'b' => 2, 'c' => 3], "dict key value")
    |> do_unset($$, keyset['a', 'b', 'c'], "keyset key value");
  var_dump($orig);
  var_dump($dict3);
}

function test($v) :mixed{
  echo "Testing: ";
  var_dump($v);
  test_append($v);
  test_set($v);
  test_setop($v);
  test_new_setop($v);
  test_unset($v);
}
<<__EntryPoint>> function main(): void {
test(dict[]);
test(dict[0 => 1, 1 => 2, 2 => 3, 3 => 4]);
test(dict["0" => 'a', "1" => 'b', "2" => 'c', "3" => 'd', "4" => 'e', "a" => 0]);
}
