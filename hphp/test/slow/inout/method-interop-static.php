<?hh

class Herp {
  function foo(inout $x) {
    $x = 42;
    echo implode(', ',array_map($a ==> $a['function'], debug_backtrace()))."\n";
    return 'Hello';
  }

  function bar(&$y) {
    $y = 9;
    echo implode(', ',array_map($a ==> $a['function'], debug_backtrace()))."\n";
    return 'world!';
  }

  static function fizz(inout $x) {
    $x = 24;
    echo implode(', ',array_map($a ==> $a['function'], debug_backtrace()))."\n";
    return 'FIZZ';
  }

  static function buzz(&$y) {
    $y = 19;
    echo implode(', ',array_map($a ==> $a['function'], debug_backtrace()))."\n";
    return 'BUZZ';
  }
}

function main() {
  $obj = new Herp;

  $a = null;
  $b = null;
  $c = null;
  $g1 = $obj->foo(inout $a);
  $g2 = $obj->foo($b);
  $g3 = $obj->foo(&$c);
  var_dump($a, $b, $c);

  $x = null;
  $y = null;
  $z = null;
  $h1 = $obj->bar($x);
  $h2 = $obj->bar(&$y);
  $h3 = $obj->bar(inout $z);
  var_dump($x, $y, $z);

  echo "$g1, $h1\n";
  echo "$g2, $h2\n";
  echo "$g2, $h2\n";

  $q = null;
  $r = null;
  $s = null;
  $f1 = Herp::fizz(inout $q);
  $f2 = Herp::fizz($r);
  $f3 = Herp::fizz(&$s);
  var_dump($q, $r, $s);

  $t = null;
  $u = null;
  $v = null;
  $k1 = Herp::buzz(inout $t);
  $k2 = Herp::buzz($u);
  $k3 = Herp::buzz(&$v);
  var_dump($t, $u, $v);

  echo "$f1, $k1\n";
  echo "$f2, $k2\n";
  echo "$f2, $k2\n";
}

main();
