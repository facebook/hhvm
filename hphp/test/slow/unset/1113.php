<?hh

function foo() :mixed{
  $a = 1;
  $b = 2;
  $c = 3;
  unset($a, $b, $c);
  try {
    var_dump($b);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}

<<__EntryPoint>>
function main_1113() :mixed{
  foo();
}
