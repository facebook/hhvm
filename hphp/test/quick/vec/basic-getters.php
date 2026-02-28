<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class ToStringObj {
  function __toString() :mixed{ return "1"; }
}

function test($v, $description) :mixed{
  $test_resource = imagecreate(1, 1);

  $tests = dict["0" => 0,
            "3" => 3,
            "8" => 8,
            "999999999" => 999999999,
            "-1" => -1,
            "\"0\"" => "0",
            "\"3\"" => "3",
            "\"foobar\"" => "foobar",
            "\"\"" => "",
            "0.0" => 0.0,
            "false" => false,
            "object" => new ToStringObj(),
            "null" => null,
            "array" => vec[1, 2, 3],
            "vec" => vec[1, 2, 3],
            "dict" => dict['1' => 1, '2' => 2, '3' => 3],
            "keyset" => keyset[1, 2, 3],
            "resource" => $test_resource,
           ];

  echo $description, ":\n";

  foreach ($tests as $str => $key) {
    try {
      echo "   \$vec[$str] => ";
      $res = $v[$key];
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   \$vec[$str] ?? \"NOT FOUND\" => ";
      $res = $v[$key] ?? "NOT FOUND";
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   idx(\$vec, $str, \"NOT FOUND\") => ";
      $res = idx($v, $key, "NOT FOUND");
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   isset(\$vec[$str]) => ";
      $res = isset($v[$key]);
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   array_key_exists(\$vec, $str) => ";
      $res = array_key_exists($key, $v);
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   empty(\$vec[$str]) => ";
      $res = !($v[$key] ?? false);
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  imagedestroy($test_resource);

  echo "foreach:\n";
  foreach ($v as $val) {
    echo "\tVal: ";
    var_dump($val);
  }

  echo "foreach with key:\n";
  foreach ($v as $key => $val) {
    echo "\tKey: ";
    var_dump($key);
    echo "\tVal: ";
    var_dump($val);
  }
}

<<__EntryPoint>> function main(): void {
  test(vec[], "empty");
  test(vec[new stdClass(), new stdClass(), new stdClass()],
       "3 objects");
  test(vec[100, 200, 300, 400], "4 ints");
  test(vec[false, true, false, true], "4 bools");
  test(vec[NULL, NULL, NULL], "3 nulls");
  test(vec['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'],
       "9 strings");
  test(vec[1, 'a', 2, 'b'], "2 ints and 2 strings");
}
