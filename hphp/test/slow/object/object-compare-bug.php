<?hh
trait T { public $z; }
class A { use T; }
class B { use T; }
function main() {
  foreach (varray[varray[1,1], varray[1,2], varray[2,1]] as list($x, $y)) {
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

<<__EntryPoint>>
function main_object_compare_bug() {
main();
}
