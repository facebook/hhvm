<?hh

function take_int(int $_): void {}

function test(bool $b): void {

  if ($b) {
    $a = vec[4];
  } else {
    $a = vec[3.14, 'aaa'];
  }

  take_int($a[1]);
}
