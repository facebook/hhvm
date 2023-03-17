<?hh

function f($val = g()) {
  echo "val = ";
  var_dump($val);
}


<<__EntryPoint>>
function main_undefined_function_default() {
f();
}
