<?hh
<<__EntryPoint>>
function main(): void {
  try {
    echo $a;
    print_r(error_get_last());
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
