<?hh

<<__EntryPoint>> function main(): void {
  $v = vec[null, darray['a' => 100], ""];
  try {
    var_dump($v->foobaz ?? 10);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
  try {
    var_dump($v?->foobaz ?? 10);
  } catch (Exception $e) {
    echo "Exception: \"" . $e->getMessage() . "\"\n";
  }
}
