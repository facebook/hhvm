<?hh

function foo(inout $x) {}

function main() {
  $x = Vector{1,array('a', 'b', 'c'),3};
  foo(inout $x[1][1]);
}


<<__EntryPoint>>
function main_bad_call_12() {
main();
}
