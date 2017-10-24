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

function main($obj, $Herp, $foo, $bar, $fizz, $buzz) {
  $a = null;
  $b = null;
  $c = null;
  $g1 = $obj->$foo(inout $a);
  $g2 = $obj->$foo($b);
  $g3 = $obj->$foo(&$c);
  var_dump($a, $b, $c);

  $x = null;
  $y = null;
  $z = null;
  $h1 = $obj->$bar($x);
  $h2 = $obj->$bar(&$y);
  $h3 = $obj->$bar(inout $z);
  var_dump($x, $y, $z);

  echo "$g1, $h1\n";
  echo "$g2, $h2\n";
  echo "$g2, $h2\n";

  $q = null;
  $r = null;
  $s = null;
  $f1 = Herp::$fizz(inout $q);
  $f2 = Herp::$fizz($r);
  $f3 = Herp::$fizz(&$s);
  var_dump($q, $r, $s);

  $t = null;
  $u = null;
  $v = null;
  $k1 = Herp::$buzz(inout $t);
  $k2 = Herp::$buzz($u);
  $k3 = Herp::$buzz(&$v);
  var_dump($t, $u, $v);

  echo "$f1, $k1\n";
  echo "$f2, $k2\n";
  echo "$f2, $k2\n";

  $q2 = null;
  $r2 = null;
  $s2 = null;
  $f4 = $Herp::$fizz(inout $q2);
  $f5 = $Herp::$fizz($r2);
  $f6 = $Herp::$fizz(&$s2);
  var_dump($q2, $r2, $s2);

  $t2 = null;
  $u2 = null;
  $v2 = null;
  $k4 = $Herp::$buzz(inout $t2);
  $k5 = $Herp::$buzz($u2);
  $k6 = $Herp::$buzz(&$v2);
  var_dump($t2, $u2, $v2);

  echo "$f4, $k4\n";
  echo "$f5, $k5\n";
  echo "$f6, $k6\n";

  $q3 = null;
  $r3 = null;
  $s3 = null;
  $f7 = $Herp::fizz(inout $q3);
  $f8 = $Herp::fizz($r3);
  $f9 = $Herp::fizz(&$s3);
  var_dump($q3, $r3, $s3);

  $t3 = null;
  $u3 = null;
  $v3 = null;
  $k7 = $Herp::$buzz(inout $t3);
  $k8 = $Herp::$buzz($u3);
  $k9 = $Herp::$buzz(&$v3);
  var_dump($t3, $u3, $v3);

  echo "$f7, $k7\n";
  echo "$f8, $k8\n";
  echo "$f9, $k9\n";
}

$obj = new Herp;
main($obj, 'Herp', 'foo', 'bar', 'fizz', 'buzz');
