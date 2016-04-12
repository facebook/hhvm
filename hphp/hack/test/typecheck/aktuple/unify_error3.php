<?hh //strict

function take_int(int $_): void {}

function test(bool $b): void {

  if ($b) {
    $a = array(4);
  } else {
    $a = array(3.14, 'aaa');
  }

  take_int($a[1]);
}
