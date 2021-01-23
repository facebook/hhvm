<?hh
<<__EntryPoint>>
function main(): void {
  error_reporting(0);
  $a = null;
  if ($a) {
    echo "a evaluates to true\n";
  } else {
    echo "a evaluates to false\n";
  }
  try {
    if ($b) {
      echo "b evaluates to true\n";
    } else {
      echo "b evaluates to false\n";
    }
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
