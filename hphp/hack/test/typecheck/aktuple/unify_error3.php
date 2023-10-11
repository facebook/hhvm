<?hh

function take_int(int $_): void {}

function test(bool $b): void {

  if ($b) {
    $a = varray[4];
  } else {
    $a = varray[3.14, 'aaa'];
  }

  take_int($a[1]);
}
