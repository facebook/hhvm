<?hh

function f(string $s): void {}

function main1(dynamic $d): void {
  f($d); // f NeedsSDT
}

function main2(string $d): void {
  f($d);
}
