<?hh


<<__EntryPoint>>
function main_no_skip_ctor() :mixed{
$rc = new ReflectionClass('HH\\StaticWaitHandle');

try {
  $wh = $rc->newInstanceWithoutConstructor();
} catch (Exception $e) {
  echo "Exception: ", $e->getMessage(), "\n";
}
}
