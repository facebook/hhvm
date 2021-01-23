<?hh
<<__EntryPoint>>
function main(): void {
  try {
    var_dump($a || null);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
