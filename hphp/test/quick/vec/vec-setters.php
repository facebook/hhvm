<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

function test_append($orig) {
  echo "Testing append....\n";
  $v = $orig;
  $v[] = 0;
  $v[] = 'a';
  $v[] = 1.23;
  $v[] = false;
  $v[] = new stdclass();
  $v[] = [300, 200, 100];
  $v[] = vec[];
  $v[] = vec[1, 2, 3];
  $v[] = dict['a' => 1, 'b' => 2, 'c' => 3];
  var_dump($orig);
  var_dump($v);
}

function do_set($vec, $key, $val) {
  try {
    $vec[$key] = $val;
    echo "Set to $val succeeded\n";
  } catch (Exception $e) {
    echo "Set to $val failed: \"", $e->getMessage(), "\"\n";
  }
  return $vec;
}

function test_set($orig) {
  echo "Testing set....\n";
  $v = do_set($orig, 0, "0 key value")
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
    |> do_set($$, new stdclass(), "object key value")
    |> do_set($$, [1, 2, 3], "array key value")
    |> do_set($$, vec[1, 2, 3], "vec key value")
    |> do_set($$, dict['a' => 1, 'b' => 2, 'c' => 3], "dict key value");
  var_dump($orig);
  var_dump($v);
}

function do_setop($vec, $key, $val) {
  try {
    $vec[$key] .= $val;
    echo "Set-Op to \"$val\" succeeded\n";
  } catch (Exception $e) {
    echo "Set-Op to \"$val\" failed: \"", $e->getMessage(), "\"\n";
  }
  return $vec;
}

function test_setop($orig) {
  echo "Test set-op....\n";
  $v = do_setop($orig, 0, " + 0 key value")
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
    |> do_setop($$, new stdclass(), " + object key value")
    |> do_setop($$, [1, 2, 3], " + array key value")
    |> do_setop($$, vec[1, 2, 3], " + vec key value")
    |> do_setop($$, dict['a' => 1, 'b' => 2, 'c' => 3], " + dict key value");
  var_dump($orig);
  var_dump($v);
}

function do_unset($vec, $key) {
  try {
    unset($vec[$key]);
    echo "Unset succeeded\n";
  } catch (Exception $e) {
    echo "Unset failed: \"", $e->getMessage(), "\"\n";
  }
  return $vec;
}

function test_unset($orig) {
  echo "Test unset....\n";
  $vec1 = do_unset($orig, 0);
  var_dump($orig);
  var_dump($vec1);

  $vec2 = do_unset($orig, 3);
  var_dump($orig);
  var_dump($vec2);

  $vec3 = do_unset($orig, 999999999)
    |> do_unset($$, -1)
    |> do_unset($$, "key1")
    |> do_unset($$, "0")
    |> do_unset($$, "3")
    |> do_unset($$, "")
    |> do_unset($$, false)
    |> do_unset($$, null)
    |> do_unset($$, 1.23)
    |> do_unset($$, new stdclass())
    |> do_unset($$, [1, 2, 3])
    |> do_unset($$, vec[1, 2, 3])
    |> do_unset($$, dict['a' => 1, 'b' => 2, 'c' => 3]);
  var_dump($orig);
  var_dump($vec3);
}

function test($v) {
  test_append($v);
  test_set($v);
  test_setop($v);
  test_unset($v);
}

test(vec[]);
test(vec[1, 2, 3, 4]);
test(vec['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k']);
