<?hh

function foo(inout $x) {}

function main() {
  static $x;
  foo(inout $x[1][2][3]);
}

main();
