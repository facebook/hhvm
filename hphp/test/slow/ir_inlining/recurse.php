<?hh


function f($x) {
  f($x);
}

<<__EntryPoint>>
function main_recurse() {
f(0);
}
