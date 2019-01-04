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

<<__EntryPoint>>
function main() {
  $obj = new Herp;

  $a = null;
  $b = null;
  $g1 = $obj->foo(inout $a);
  $g2 = $obj->foo(&$b);
  var_dump($a, $b);

  $x = null;
  $y = null;
  $h1 = $obj->bar(&$x);
  $h2 = $obj->bar(inout $y);
  var_dump($x, $y);

  echo "$g1, $h1\n";
  echo "$g2, $h2\n";

  $q = null;
  $r = null;
  $f1 = Herp::fizz(inout $q);
  $f2 = Herp::fizz(&$r);
  var_dump($q, $r);

  $t = null;
  $u = null;
  $k1 = Herp::buzz(inout $t);
  $k2 = Herp::buzz(&$u);
  var_dump($t, $u);

  echo "$f1, $k1\n";
  echo "$f2, $k2\n";
}
