<?hh

function take_float(float $_): void {}

function test(bool $b): void {

  if ($b) {
    $a = varray[4];
  } else {
    $a = varray[3.14, 'aaa'];
  }

  take_float($a[0]);
}
