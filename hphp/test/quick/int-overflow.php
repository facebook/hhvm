<?hh

class C {
  public $max = PHP_INT_MAX;
}

function error_boundary(inout $x, $fn) :mixed{
  try {
    $fn(inout $x);
  } catch (Exception $e) {
    print("Error: ".$e->getMessage()."\n");
  }
}

<<__EntryPoint>> function main(): void {
  $add = function($a,$b) { return HH\Lib\Legacy_FIXME\cast_for_arithmetic($a) + HH\Lib\Legacy_FIXME\cast_for_arithmetic($b); };
  $sub = function($a,$b) { return $a - $b; };
  $mul = function($a,$b) { return $a * $b; };

  $max = PHP_INT_MAX;
  $min = 1 << 63;

  // some initial cases for the simplifier
  var_dump($max + 1);
  var_dump($min - 1);
  var_dump($max * 5);

  $ops = vec[
    // initial sanity checks for no overflow
    vec[$add, 3, 5],
    vec[$sub, 3, 5],
    vec[$mul, 7, 4],

    // check runtime operators on just ints
    vec[$add, $max, 1],
    vec[$add, $min, -1],

    vec[$sub, $min, 1],
    vec[$sub, $max, -1],

    vec[$mul, $max / 2, 3],
    vec[$mul, $min, 2],
    vec[$mul, $max, -2],
    vec[$mul, $min, -3],

    // check numeric strings
    vec[$add, "$max", 1],
    vec[$add, $max, '1'],
    vec[$add, "$max", '1'],
  ];

  foreach ($ops as list($op, $lhs, $rhs)) {
    $res = $op($lhs, $rhs);
    var_dump($res);
  }

  $unary = vec[$min, $max, -4, 0, 5, "12", 5.2, "1.5", "abc", "", null];

  // inc/dec
  foreach ($unary as $val) {
    $x = $val;
    error_boundary(inout $x, (inout $o) ==> var_dump(++$o));
    var_dump($x);

    $x = $val;
    error_boundary(inout $x, (inout $o) ==> var_dump($o++));
    var_dump($x);

    $x = $val;
    error_boundary(inout $x, (inout $o) ==> var_dump(--$o));
    var_dump($x);

    $x = $val;
    error_boundary(inout $x, (inout $o) ==> var_dump($o--));
    var_dump($x);
  }

  // arrays
  $array = $unary;
  for ($i = 0; $i < count($array); ++$i) {
    var_dump($array[$i]);
    error_boundary(inout $array[$i], (inout $o) ==> var_dump($o++));
    var_dump($array[$i]);
  }

  $array = $unary;
  for ($i = 0; $i < count($array); ++$i) {
    var_dump($array[$i]);
    error_boundary(inout $array[$i], (inout $o) ==> var_dump(++$o));
    var_dump($array[$i]);
  }

  $array = $unary;
  for ($i = 0; $i < count($array); ++$i) {
    var_dump($array[$i]);
    error_boundary(inout $array[$i], (inout $o) ==> var_dump($o--));
    var_dump($array[$i]);
  }

  $array = $unary;
  for ($i = 0; $i < count($array); ++$i) {
    var_dump($array[$i]);
    error_boundary(inout $array[$i], (inout $o) ==> var_dump(--$o));
    var_dump($array[$i]);
  }

  // properties

  $c = new C;
  var_dump($c->max);
  var_dump($c->max++);
  var_dump($c->max);

  $c = new C;
  var_dump($c->max);
  var_dump(++$c->max);
  var_dump($c->max);

  $c = new C;
  var_dump($c->max);
  var_dump($c->max--);
  var_dump($c->max);

  $c = new C;
  var_dump($c->max);
  var_dump(--$c->max);
  var_dump($c->max);
}
