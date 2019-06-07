<?hh

function g() {
  throw new Exception("Fooception");
  return "Oh uh...";
}

function f($val = g()) {
  echo "val = ";
  var_dump($val);
}


<<__EntryPoint>>
function main_exception_default() {
try {
  f();
} catch (Exception $e) {
  echo "Caught exception: {$e->getMessage()}\n";
}
}
