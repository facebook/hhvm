<?hh

class C {}

function test_dyn_array(dynamic $d, C $c) : void {
  $d[1] = $c;
  $d[$d] = $c;
  $d[] = $c;
  $d[$c];
  $d[$c] = 1;
}
