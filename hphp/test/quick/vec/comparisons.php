<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

function single_compare($a, $b) :mixed{
  echo "Comparing:\n";
  var_dump($a);
  var_dump($b);

  echo "\t<   : ";
  try {
    var_dump(HH\Lib\Legacy_FIXME\lt($a, $b));
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t<=  : ";
  try {
    var_dump(HH\Lib\Legacy_FIXME\lte($a, $b));
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t>   : ";
  try {
    var_dump(HH\Lib\Legacy_FIXME\gt($a, $b));
  } catch (Exception $e) {
    echo "Error: ", $e->getMessage(), "\n";
  }

  echo "\t>=  : ";
  try {
    var_dump(HH\Lib\Legacy_FIXME\gte($a, $b));
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
    var_dump(HH\Lib\Legacy_FIXME\cmp($a, $b));
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
  single_compare(vec[], vec[]);
  single_compare(vec[1, 2, 3], vec[1, 2, 3]);
  compare(vec[4, 5], vec[4, 6]);
  compare(vec[], vec[1, 2]);
  compare(vec[1, 2, 3], vec[3, 2, 1]);

  $v = vec['a', 'b'];
  single_compare($v, $v);

  compare(vec[12345], vec["12345"]);

  single_compare(vec[new stdClass], vec[new stdClass]);
  compare(vec[new ToString('foobaz')], vec['foobaz']);
  compare(vec[new Thrower], vec['foobaz']);
  compare(vec[1, new Thrower], vec[2, 'foobaz']);

  compare(vec[], null);
  compare(vec[], false);
  compare(vec[], 123);
  compare(vec[], 1.2345);
  compare(vec[], 'abc');
  compare(vec[], new stdClass);
  compare(vec[], vec[]);
  compare(vec[], dict[]);
  compare(vec[], keyset[]);

  single_compare(vec[dict[]], vec[dict[]]);
}
