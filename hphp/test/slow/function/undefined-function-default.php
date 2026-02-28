<?hh

function f($val = g()) :mixed{
  echo "val = ";
  var_dump($val);
}


<<__EntryPoint>>
function main_undefined_function_default() :mixed{
f();
}
