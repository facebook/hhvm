<?hh
<<__EntryPoint>>
function main(): void {
  $a = darray[];
  $a[0] = varray[$a];
  var_dump($a);
  try {
    $b = varray[varray[$b]];
    var_dump($b);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
  try {
    $c = varray[$c];
    var_dump($c);
  } catch (UndefinedVariableException $e) {
    var_dump($e->getMessage());
  }
}
