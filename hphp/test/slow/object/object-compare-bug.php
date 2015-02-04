<?php
trait T { public $z; }
class A { use T; }
class B { use T; }
function main() {
  foreach (array(array(1,1), array(1,2), array(2,1)) as list($x, $y)) {
    $a = new A;
    $a->z = $x;
    $b = new B;
    $b->z = $y;
    var_dump($a == $b);
    var_dump($b == $a);
    var_dump($a != $b);
    var_dump($b != $a);
    var_dump($a < $b);
    var_dump($b < $a);
    var_dump($a <= $b);
    var_dump($b <= $a);
    var_dump($a > $b);
    var_dump($b > $a);
    var_dump($a >= $b);
    var_dump($b >= $a);
    echo "--------\n";
  }
}
main();
