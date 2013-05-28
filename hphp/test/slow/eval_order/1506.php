<?php

function x($a, $b, $c, $d) {
}
function p($x) {
 echo $x . "
";
 return $x;
 }
class c {
  function __construct($a, $b, $c, $d) {
}
  function f($a, $b, $c, $d) {
}
  static function g($a, $b, $c, $d) {
}
}
function rt(&$a, $v) {
  $a = $v;
}
function id($x) {
 return $x;
 }
function dump($a, $b) {
  var_dump($a, $b);
}
echo "sfc
";
x(p(1), p(2), p(3), 4);
$y = 'x';
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
rt($a, id(10));
var_dump($a);
dump($v++, $v++);
$v = 10;
dump($v, $v = 0);
echo "nest
";
x(p(1), x(p(2), p(3), p(4), p(5)), p(6), x(p(7), p(8), p(9), p(10)));
echo "arr
";
$z = array(p(1), p(2), x(p(3), p(4), p(5), p(6)), p(7));
$q = 1;
$z = array(1, 2, $q);
