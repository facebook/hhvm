<?hh

<<__EntryPoint>> function autoload_017(): void {
$rc = new ReflectionClass("stdClass");

try {
  $rc->implementsInterface("UndefI");
} catch (ReflectionException $e) {
  echo $e->getMessage();
}
}
