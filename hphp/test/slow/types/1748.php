<?hh

function foo($p) {
  if ($p) {
    $a = varray[];
  }
  try {
    var_dump((string)$a);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1748() {
  foo(false);
}
