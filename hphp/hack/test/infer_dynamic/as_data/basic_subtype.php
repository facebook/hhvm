<?hh

function takes_int(int $_): void {}
function takes_arraykey(arraykey$_): void {}

function test_basic(dynamic $d): void {
  takes_int($d);
  takes_arraykey($d);
}
