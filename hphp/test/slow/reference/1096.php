<?hh
<<__EntryPoint>>
function main(): void {
  $a = dict[];
  $a[0] = vec[$a];
  var_dump($a);
  try {
    $b = vec[vec[$b]];
    var_dump($b);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    $c = vec[$c];
    var_dump($c);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
