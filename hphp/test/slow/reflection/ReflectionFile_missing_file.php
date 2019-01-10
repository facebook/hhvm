<?hh // strict

try {
  new ReflectionFile('NoSuchFile');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}
