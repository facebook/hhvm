<?hh //strict

function take_num(num $_): void {}
function take_string(string $_): void {}

function test(bool $b): void {

  if ($b) {
    $a = array(4);
  } else {
    $a = array(3.14, 'aaa');
  }

  take_num($a[0]);
  take_string($a[1]);
}
