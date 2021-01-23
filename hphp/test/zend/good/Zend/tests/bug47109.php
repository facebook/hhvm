<?hh
<<__EntryPoint>>
function main(): void {
  try {
    $a->{"a"."b"};
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
