<?hh

function takes_int(int $_): void {}
function takes_arraykey(arraykey$_): void {}

function test_if_else(dynamic $d, bool $b): void {
  if ($b) {
    takes_int($d);
  } else {
    takes_arraykey($d);
  }
}
