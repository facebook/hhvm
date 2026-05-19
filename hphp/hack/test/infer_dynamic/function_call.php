<?hh

function takes_int(int $_): void {}

function test_call_dynamic(dynamic $d): void {
  $r = $d(42, "hello");
  takes_int($r);
}
