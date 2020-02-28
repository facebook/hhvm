<?hh

function foo() {
  $x = darray[];
  $x[new stdclass] = 2;
  var_dump($x);
}

<<__EntryPoint>>
function main_array_055() {
foo();
}
