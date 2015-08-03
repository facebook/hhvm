<?hh

try {
  $x = new ReflectionTypeAlias('NoSuchTypeAlias');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}
