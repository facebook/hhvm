<?hh

function foo(inout $x) :mixed{}

function main() :mixed{
  $x = Vector{1,vec['a', 'b', 'c'],3};
  foo(inout $x[1][1]);
}


<<__EntryPoint>>
function main_bad_call_12() :mixed{
main();
}
