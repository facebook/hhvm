<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function append_via_ref(inout $ks, $val) :mixed{
  $ks[] = $val;
}

function do_append($ks, $val, $s, $via_ref) :mixed{
  try {
    if ($via_ref) {
      append_via_ref(inout $ks, $val);
    } else {
      $ks[] = $val;
    }
    echo "Append of $s succeeded\n";
  } catch (Exception $e) {
    echo "Append of $s failed: \"", $e->getMessage(), "\"\n";
  }
  return $ks;
}

function test_append($orig, $via_ref) :mixed{
  echo "Testing append".($via_ref ? " via ref" : "")."....\n";
  $v = do_append($orig, 0, "0 integer", $via_ref)
    |> do_append($$, 2, "2 integer", $via_ref)
    |> do_append($$, -5, "-5 integer", $via_ref)
    |> do_append($$, "a", "\"a\" string", $via_ref)
    |> do_append($$, "foobaz", "\"foobaz\" string", $via_ref)
    |> do_append($$, 123, "123 integer", $via_ref)
    |> do_append($$, "123", "\"123\" string", $via_ref)
    |> do_append($$, "", "empty string", $via_ref)
    |> do_append($$, "2", "\"2\" integer", $via_ref)
    |> do_append($$, 0, "0 integer (again)", $via_ref)
    |> do_append($$, 1.23, "double", $via_ref)
    |> do_append($$, false, "bool", $via_ref)
    |> do_append($$, null, "null", $via_ref)
    |> do_append($$, new stdClass(), "class", $via_ref)
    |> do_append($$, vec[300, 200, 100], "array", $via_ref)
    |> do_append($$, vec[1, 2, 3], "vec", $via_ref)
    |> do_append($$, dict['a' => 1, 'b' => 2], "dict", $via_ref)
    |> do_append($$, keyset['a', 'b', 'c'], "keyset", $via_ref);
  var_dump($orig);
  var_dump($v);
}

function do_set($ks, $key, $val) :mixed{
  try {
    $ks[$key] = $val;
    echo "Set to $val succeeded\n";
  } catch (Exception $e) {
    echo "Set to $val failed: \"", $e->getMessage(), "\"\n";
  }
  return $ks;
}

function test_set($orig) :mixed{
  echo "Testing set....\n";
  $v = do_set($orig, 0, "0 key value")
    |> do_set($$, 3, "3 key value")
    |> do_set($$, 999999999, "999999999 key value")
    |> do_set($$, -1, "-1 key value")
    |> do_set($$, "a", "\"a\" key value")
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
  var_dump($v);
}

function do_setop($ks, $key, $val) :mixed{
  try {
    $ks[$key] .= $val;
    echo "Set-Op with \"$val\" succeeded\n";
  } catch (Exception $e) {
    echo "Set-Op with \"$val\" failed: \"", $e->getMessage(), "\"\n";
  }
  return $ks;
}

function test_setop($orig) :mixed{
  echo "Test set-op....\n";
  $v = do_setop($orig, 0, " + 0 key value")
    |> do_setop($$, 3, " + 3 key value")
    |> do_setop($$, 999999999, " + 999999999 key value")
    |> do_setop($$, -1, " + -1 key value")
    |> do_setop($$, "a", " + \"a\" key value")
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
  var_dump($v);
}

function do_new_setop($ks, $val, $s) :mixed{
  try {
    $ks[] .= $val;
    echo "New set-op with \"$s\" succeeded\n";
  } catch (Exception $e) {
    echo "New set-op with \"$s\" failed: \"", $e->getMessage(), "\"\n";
  }
  return $ks;
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

function do_unset($ks, $key, $str) :mixed{
  try {
    unset($ks[$key]);
    echo "Unset of $str succeeded\n";
  } catch (Exception $e) {
    echo "Unset of $str failed: \"", $e->getMessage(), "\"\n";
  }
  return $ks;
}

function test_unset($orig) :mixed{
  echo "Test unset....\n";
  $ks1 = do_unset($orig, 2, "2 key value");
  var_dump($orig);
  var_dump($ks1);

  $ks2 = do_unset($orig, 3, "3 key value");
  var_dump($orig);
  var_dump($ks2);

  $ks3 = do_unset($orig, "b", "\"b\" key value");
  var_dump($orig);
  var_dump($ks3);

  $ks4 = do_unset($orig, "3", "\"3\" value value");
  var_dump($orig);
  var_dump($ks4);

  $ks5 = do_unset($orig, 999999999, "999999999 key value")
    |> do_unset($$, -1, "-1 key value")
    |> do_unset($$, "key1", "\"key1\" key value")
    |> do_unset($$, "0", "\"0\" key value")
    |> do_unset($$, "1", "\"1\" key value")
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
  var_dump($ks5);
}

function test($v) :mixed{
  echo "Testing: ";
  var_dump($v);
  test_append($v, false);
  test_append($v, true);
  test_set($v);
  test_setop($v);
  test_new_setop($v);
  test_unset($v);
}
<<__EntryPoint>> function main(): void {
test(keyset[]);
test(keyset[1, 2, 3, 4]);
test(keyset['b', '3', 3, 10, 'e', 'f', 'g']);
}
