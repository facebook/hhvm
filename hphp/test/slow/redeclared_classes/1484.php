<?hh

if (isset($g)) {
  include '1484-1.inc';
}
else {
  include '1484-2.inc';
}
class X1 extends X {
  public $t = 1;
}
function test() {
  $x = new X1;
  $x->t = 5;
  $x->a = 3;
  $y = clone $x;
  var_dump($y->a,$y->t);
}
test();
