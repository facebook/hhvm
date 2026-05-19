<?hh

function takes_int(int $_): void {}

function test_chain(dynamic $d): void {
  $a = $d['key'];
  takes_int($a);
}
