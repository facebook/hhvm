<?hh

function foo(inout $x) {}

function main() {
  static $x;
  foo(inout $x[1][2][3]);
}


<<__EntryPoint>>
function main_bad_call_11() {
main();
}
