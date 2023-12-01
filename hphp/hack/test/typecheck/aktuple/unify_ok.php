<?hh

function take_num(num $_): void {}
function take_string(string $_): void {}

function test(bool $b): void {

  if ($b) {
    $a = vec[4];
  } else {
    $a = vec[3.14, 'aaa'];
  }

  take_num($a[0]);
  take_string($a[1]);
}
