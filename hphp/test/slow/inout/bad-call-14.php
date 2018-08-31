<?hh

function foo(inout $x) {}

function main() {
  $x = Vector{1,2,3};
  foo(inout $x[1]);
}


<<__EntryPoint>>
function main_bad_call_14() {
main();
}
