<?hh

$rc = new ReflectionClass('HH\\WaitHandle');

try {
  $wh = $rc->newInstanceWithoutConstructor();
} catch (Exception $e) {
  echo "Exception: ", $e->getMessage(), "\n";
}
