<?hh
<<__EntryPoint>> function main(): void {
$a = null;
try {
  $a += new Exception();
  var_dump($a);
} catch (Exception $e) {
  var_dump($e->getMessage());
}
}
