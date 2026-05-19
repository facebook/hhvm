<?hh

function takes_int(int $_): void {}

function test_chain_member(dynamic $d): void {
  $x = $d->foo;
  takes_int($x);
}
