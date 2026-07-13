<?hh


<<__EntryPoint>>
function main_1516() :mixed{
  try {
    $t1 = $v; $v++; $t2 = $v; $v++; var_dump($t1, $t2);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
