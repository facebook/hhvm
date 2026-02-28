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
      echo "   \$dict[$str] => ";
      $res = $v[$key];
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   \$dict[$str] ?? \"NOT FOUND\" => ";
      $res = $v[$key] ?? "NOT FOUND";
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   idx(\$dict, $str, \"NOT FOUND\") => ";
      $res = idx($v, $key, "NOT FOUND");
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   isset(\$dict[$str]) => ";
      $res = isset($v[$key]);
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   array_key_exists(\$dict, $str) => ";
      $res = array_key_exists($key, $v);
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   empty(\$dict[$str]) => ";
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
  test(dict[], "empty");
  test(dict[0 => new stdClass(), 1 => new stdClass(), 2 => new stdClass()],
       "3 objects");
  test(dict[0 => 100, 1 => 200, 2 => 300, 3 => 400], "4 ints");
  test(dict[0 => false, 1 => true, 2 => false, 3 => true], "4 bools");
  test(dict[0 => NULL, 1 => NULL, 2 => NULL], "3 nulls");
  test(dict["0" => 'a', "1" => 'b', "2" => 'c', "3" => 'd', "4" => 'e'],
       "5 strings");
  test(dict[0 => "val1", "key1" => 100, 1 => 200, "key2" => "val2"],
       "int/string mix");
}
