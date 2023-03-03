<?hh

function f(string $s): void {}

function main1(string $d): void {
  f($d);
}

function main2(dynamic $d): void {
  f($d); // f NeedsSDT
}
