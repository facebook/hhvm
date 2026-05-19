<?hh

function takes_int(int $_): void {}
function takes_string(string $_): void {}

function test_foreach(dynamic $d): void {
  foreach ($d as $v) {
    takes_int($v);
  }
}

function test_foreach_kv(dynamic $d): void {
  foreach ($d as $k => $v) {
    takes_string($k);
    takes_int($v);
  }
}
