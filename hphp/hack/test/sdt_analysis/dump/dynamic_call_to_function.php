<?hh

function f(string $_): void {}

function main(dynamic $d): void {
  f($d); // f NeedsSDT
}
