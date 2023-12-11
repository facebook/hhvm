<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function single_compare($a, $b) :mixed{
  echo "Comparing:\n";
  var_dump($a);
  var_dump($b);

  echo "\t<   : ";
  try {
    var_dump($a < $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t<=  : ";
  try {
    var_dump($a <= $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t>   : ";
  try {
    var_dump($a > $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t>=  : ";
  try {
    var_dump($a >= $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t==  : ";
  try {
    var_dump(HH\Lib\Legacy_FIXME\eq($a, $b));
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t!=  : ";
  try {
    var_dump(HH\Lib\Legacy_FIXME\neq($a, $b));
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t=== : ";
  try {
    var_dump($a === $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t!== : ";
  try {
    var_dump($a !== $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t<=> : ";
  try {
    var_dump($a <=> $b);
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }
}

class ToString {
  public $str;
  function __construct($str) {
    $this->str = $str;
  }
  function __toString()[] :mixed{
    return $this->str;
  }
}

class Thrower {
  function __toString()[] :mixed{
    throw new Exception("Compare exception");
  }
}

function compare($a, $b) :mixed{
  single_compare($a, $b);
  single_compare($b, $a);
}

<<__EntryPoint>> function main(): void {
  single_compare(dict[], dict[]);
  single_compare(dict[0 => 1, 1 => 2, 2 => 3],
                 dict[0 => 1, 1 => 2, 2 => 3]);
  single_compare(dict['a' => 1, 'b' => 2, 'c' => 3],
                 dict['a' => 1, 'b' => 2, 'c' => 3]);
  single_compare(dict[0 => 1, 'b' => 2, 3 => 'c', 'd' => 'e'],
                 dict[0 => 1, 'b' => 2, 3 => 'c', 'd' => 'e']);
  compare(dict[0 => 4, 1 => 5],
          dict[0 => 4, 1 => 6]);
  compare(dict[0 => 4, 1 => 5],
          dict[0 => 4, 2 => 5]);
  compare(dict[], dict[0 => 1, 'key' => 'value']);
  compare(dict['a' => 0], dict['b' => 0]);
  compare(dict[0 => 'a', 1 => 'b', 2 => 'c'],
          dict[2 => 'c', 1 => 'b', 0 => 'a']);

  $d = dict[0 => 'a', 5 => 'b'];
  single_compare($d, $d);

  compare(dict['a' => 12345], dict['a' => "12345"]);

  single_compare(dict[0 => new stdClass],
                 dict[0 => new stdClass]);
  compare(dict[0 => new ToString('foobaz')],
          dict[0 => 'foobaz']);
  compare(dict["key" => new Thrower],
          dict["key" => 'foobaz']);
  compare(dict[0 => new Thrower],
          dict[1 => 'foobaz']);
  compare(dict['a' => 1, 'b' => new Thrower],
          dict['a' => 2, 'b' => 'foobaz']);

  compare(dict[], null);
  compare(dict[], false);
  compare(dict[], 123);
  compare(dict[], 1.2345);
  compare(dict[], 'abc');
  compare(dict[], new stdClass);
  compare(dict[], vec[]);
  compare(dict[], vec[]);
  compare(dict[], keyset[]);

  single_compare(dict[0 => keyset[]], dict[0 => keyset[]]);
}
