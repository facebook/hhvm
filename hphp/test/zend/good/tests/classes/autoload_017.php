<?hh

<<__EntryPoint>> function main(): void {
$rc = new ReflectionClass("stdClass");

try {
  $rc->implementsInterface("UndefI");
} catch (ReflectionException $e) {
  echo $e->getMessage();
}
}
