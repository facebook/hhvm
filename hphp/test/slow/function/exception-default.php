<?hh

function g() :mixed{
  throw new Exception("Fooception");
  return "Oh uh...";
}

function f($val = g()) :mixed{
  echo "val = ";
  var_dump($val);
}


<<__EntryPoint>>
function main_exception_default() :mixed{
try {
  f();
} catch (Exception $e) {
  echo "Caught exception: {$e->getMessage()}\n";
}
}
