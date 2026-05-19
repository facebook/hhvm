<?hh

function takes_int(int $_): void {}

function test_while_loop(dynamic $d, bool $b): void {
  while ($b) {
    takes_int($d);
  }
}
