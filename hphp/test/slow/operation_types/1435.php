<?hh
<<__EntryPoint>> function main(): void {
$a = null;
try {
  $a = HH\Lib\Legacy_FIXME\cast_for_arithmetic($a);
  $a += HH\Lib\Legacy_FIXME\cast_for_arithmetic(new Exception());
  var_dump($a);
} catch (TypecastException $e) {
  var_dump($e->getMessage());
}
}
