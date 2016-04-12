<?hh //strict

function take_float(float $_): void {}

function test(bool $b): void {

  if ($b) {
    $a = array(4);
  } else {
    $a = array(3.14, 'aaa');
  }

  take_float($a[0]);
}
