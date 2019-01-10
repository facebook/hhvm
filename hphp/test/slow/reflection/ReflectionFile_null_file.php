<?hh // strict

try {
  new ReflectionFile(null);
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}
