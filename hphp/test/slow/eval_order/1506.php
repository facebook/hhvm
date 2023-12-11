<?hh
<<__DynamicallyCallable>>
function x($a, $b, $c, $d) :mixed{
}
function p($x) :mixed{
  echo $x.
    "
";
  return $x;
}
class c {
  function __construct($a, $b, $c, $d) {
  }
  function f($a, $b, $c, $d) :mixed{
  }
  <<__DynamicallyCallable>>
  static function g($a, $b, $c, $d) :mixed{
  }
}
function rt(inout $a, $v) :mixed{
  $a = $v;
}
function id($x) :mixed{
  return $x;
}
function dump($a, $b) :mixed{
  var_dump($a, $b);
}

<<__EntryPoint>>
function main_1506() :mixed{
  echo "sfc
";
  x(p(1), p(2), p(3), 4);
  $y = x<>;
  echo "dfc
";
  $y(p(1), p(2), p(3), 4);
  echo "smc
";
  c::g(p(1), p(2), p(3), 4);
  $y = 'g';
  echo "dsmc
";
  c::$y(p(1), p(2), p(3), 4);
  echo "occ
";
  $q = new c(p(1), p(2), p(3), 4);
  echo "omc
";
  $q->f(p(1), p(2), p(3), 4);
  echo "rsfc
";
  $a = null;
  rt(inout $a, id(10));
  var_dump($a);
  try {
    dump($v++, $v++);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  $v = 10;
  dump($v, $v = 0);
  echo "nest
";
  x(p(1), x(p(2), p(3), p(4), p(5)), p(6), x(p(7), p(8), p(9), p(10)));
  echo "arr
";
  $z = vec[p(1), p(2), x(p(3), p(4), p(5), p(6)), p(7)];
  $q = 1;
  $z = vec[1, 2, $q];
}
