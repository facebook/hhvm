<?hh

function foo(inout $x) :mixed{}

function main() :mixed{
  $x = Vector{1,2,3};
  foo(inout $x[1]);
}


<<__EntryPoint>>
function main_bad_call_14() :mixed{
main();
}
