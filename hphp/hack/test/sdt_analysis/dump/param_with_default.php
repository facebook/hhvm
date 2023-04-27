<?hh

function f(
  string $_,
  string $default = "how do you like them apples"
): void {}

function main(dynamic $dyn): void {
  f($dyn);
}
