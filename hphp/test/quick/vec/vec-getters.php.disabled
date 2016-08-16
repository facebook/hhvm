<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class ToStringObj {
  function __toString() { return '1'; }
}

function test($v, $description) {
  $test_resource = imagecreate(1, 1);

  $tests = ["0" => 0,
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
            "array" => [1, 2, 3],
            "vec" => vec[1, 2, 3],
            "dict" => dict['1' => 1, '2' => 2, '3' => 3],
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

  imagedestroy($test_resource);
}

function main() {
  test(vec[], "empty");
  test(vec[new stdclass(), new stdclass(), new stdclass()],
       "3 objects");
  test(vec[100, 200, 300, 400], "4 ints");
  test(vec['a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i'],
       "9 strings");
}

main();
