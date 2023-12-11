<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class ToStringObj {
  function __toString() :mixed{ return "1"; }
}

function test($k, $description) :mixed{
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
      echo "   \$keyset[$str] => ";
      $res = $k[$key];
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   \$keyset[$str] ?? \"NOT FOUND\" => ";
      $res = $k[$key] ?? "NOT FOUND";
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   idx(\$keyset, $str, \"NOT FOUND\") => ";
      $res = idx($k, $key, "NOT FOUND");
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   isset(\$keyset[$str]) => ";
      $res = isset($k[$key]);
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   array_key_exists(\$keyset, $str) => ";
      $res = array_key_exists($key, $k);
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  foreach ($tests as $str => $key) {
    try {
      echo "   empty(\$keyset[$str]) => ";
      $res = !($k[$key] ?? false);
      var_dump($res);
    } catch (Exception $e) {
      echo "<Exception: \"", $e->getMessage(), "\">\n";
    }
  }

  imagedestroy($test_resource);

  echo "foreach:\n";
  foreach ($k as $val) {
    echo "\tVal: ";
    var_dump($val);
  }

  echo "foreach with key:\n";
  foreach ($k as $key => $val) {
    echo "\tKey: ";
    var_dump($key);
    echo "\tVal: ";
    var_dump($val);
  }
}

<<__EntryPoint>> function main(): void {
  test(keyset[], "empty");
  test(keyset[0, 100, 3, 400], "4 ints");
  test(keyset["foobar", "A", "3", "", "B"], "5 strings");
  test(keyset[0, "A", 1, "B"], "2 ints, 2 strings");
}
