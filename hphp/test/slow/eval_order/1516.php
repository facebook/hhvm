<?hh


<<__EntryPoint>>
function main_1516() {
  try {
    var_dump($v++, $v++);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
