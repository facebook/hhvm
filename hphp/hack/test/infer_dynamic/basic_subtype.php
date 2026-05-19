<?hh

function takes_int(int $_): void {}
function takes_string(string $_): void {}

function test_basic(dynamic $d): void {
  takes_int($d);
  takes_string($d);
}
