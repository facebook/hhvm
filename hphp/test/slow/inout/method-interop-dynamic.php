<?hh

class Herp {
  function foo(inout $x) {
    $x = 42;
    $bt = array_slice(debug_backtrace(), 0, 2);
    echo implode(', ', array_map($a ==> $a['function'], $bt))."\n";
    return 'Hello';
  }

  function bar(&$y) {
    $y = 9;
    $bt = array_slice(debug_backtrace(), 0, 2);
    echo implode(', ', array_map($a ==> $a['function'], $bt))."\n";
    return 'world!';
  }

  static function fizz(inout $x) {
    $x = 24;
    $bt = array_slice(debug_backtrace(), 0, 2);
    echo implode(', ', array_map($a ==> $a['function'], $bt))."\n";
    return 'FIZZ';
  }

  static function buzz(&$y) {
    $y = 19;
    $bt = array_slice(debug_backtrace(), 0, 2);
    echo implode(', ', array_map($a ==> $a['function'], $bt))."\n";
    return 'BUZZ';
  }
}

function main($obj, $Herp, $foo, $bar, $fizz, $buzz) {
  $a = null;
  $b = null;
  $c = null;
  $g1 = $obj->$foo(inout $a);
  $g2 = $obj->$foo(&$b);
  var_dump($a, $b);

  $x = null;
  $y = null;
  $h1 = $obj->$bar(&$x);
  $h2 = $obj->$bar(inout $y);
  var_dump($x, $y);

  echo "$g1, $h1\n";
  echo "$g2, $h2\n";

  $q = null;
  $r = null;
  $f1 = Herp::$fizz(inout $q);
  $f2 = Herp::$fizz(&$r);
  var_dump($q, $r);

  $t = null;
  $u = null;
  $k1 = Herp::$buzz(inout $t);
  $k2 = Herp::$buzz(&$u);
  var_dump($t, $u);

  echo "$f1, $k1\n";
  echo "$f2, $k2\n";

  $q2 = null;
  $r2 = null;
  $s2 = null;
  $f4 = $Herp::$fizz(inout $q2);
  $f5 = $Herp::$fizz(&$r2);
  var_dump($q2, $r2);

  $t2 = null;
  $u2 = null;
  $k4 = $Herp::$buzz(inout $t2);
  $k5 = $Herp::$buzz(&$u2);
  var_dump($t2, $u2);

  echo "$f4, $k4\n";
  echo "$f5, $k5\n";

  $q3 = null;
  $r3 = null;
  $f7 = $Herp::fizz(inout $q3);
  $f8 = $Herp::fizz(&$r3);
  var_dump($q3, $r3);

  $t3 = null;
  $u3 = null;
  $k7 = $Herp::$buzz(inout $t3);
  $k8 = $Herp::$buzz(&$u3);
  var_dump($t3, $u3);

  echo "$f7, $k7\n";
  echo "$f8, $k8\n";
}


<<__EntryPoint>>
function main_method_interop_dynamic() {
  $obj = new Herp;
  main($obj, 'Herp', 'foo', 'bar', 'fizz', 'buzz');
}
