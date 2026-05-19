<?hh

function takes_int(int $_): void {}
function takes_string(string $_): void {}

function test_array_read(dynamic $d): void {
  $a = $d['key1'];
  takes_int($a);
  $a = $d['key2'];
  takes_string($a);
}
