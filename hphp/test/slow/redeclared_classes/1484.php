<?hh

function test() :mixed{
  $x = new X1;
  $x->t = 5;
  $x->a = 3;
  $y = clone $x;
  var_dump($y->a,$y->t);
}
<<__EntryPoint>>
function entrypoint_1484(): void {

  if (isset($g)) {
    include '1484-1.inc';
  }
  else {
    include '1484-2.inc';
  }
  include '1484-classes.inc';
  test();
}
