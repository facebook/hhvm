<?hh

function takes_string(string $_): void {}

function test_method_call(dynamic $d): void {
  $r = $d->bar(42);
  takes_string($r);
}
