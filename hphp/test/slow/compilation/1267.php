<?hh

<<__EntryPoint>>
function main_1267() :mixed{
  try {
    $a = 0xC0000000 & $b;
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
