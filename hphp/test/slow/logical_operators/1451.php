<?hh
function foo() :mixed{
  echo "foo";
}
<<__EntryPoint>> function main(): void {
$a = (false && foo());
$b = (true  || foo());
var_dump($a, $b);
try {
  $c = ($c || true);
  var_dump($c);
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
try {
  $d = ($d && false);
  var_dump($d);
} catch (UndefinedVariableException $e) {
  var_dump($e->getMessage());
}
}
