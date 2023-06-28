<?hh


function f($x) :mixed{
  f($x);
}

<<__EntryPoint>>
function main_recurse() :mixed{
f(0);
}
