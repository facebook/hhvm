<?hh
<<__EntryPoint>> function main(): void {
$a = null;
try {
  $a += new Exception();
  var_dump($a);
} catch (TypecastException $e) {
  var_dump($e->getMessage());
}
}
