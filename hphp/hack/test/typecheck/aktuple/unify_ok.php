<?hh //strict

function take_num(num $_): void {}
function take_string(string $_): void {}

function test(bool $b): void {

  if ($b) {
    $a = varray[4];
  } else {
    $a = varray[3.14, 'aaa'];
  }

  take_num($a[0]);
  take_string($a[1]);
}
