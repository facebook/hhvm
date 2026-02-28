<?hh

function foo($p) :mixed{
  if ($p) {
    $a = vec[];
  }
  try {
    var_dump((string)$a);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1748() :mixed{
  foo(false);
}
