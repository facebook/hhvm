<?hh
<<__EntryPoint>>
function main(): void {
  error_reporting(E_ALL & ~E_NOTICE);
  try {
    echo "blah-$foo\n";
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
