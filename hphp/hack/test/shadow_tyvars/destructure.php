<?hh

function takes_int(int $_): void {}
function takes_string(string $_): void {}

function test_destructure(dynamic $d): void {
  list($a, $b) = $d;
  takes_int($a);
  takes_string($b);
}
