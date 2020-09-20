<?hh

function f($x) {
  var_dump(is_array($x), $x[0]);
}

<<__EntryPoint>>
function main_1861() {
f(varray[0]);
f('foo');
}
