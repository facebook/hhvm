<?hh // strict

try {
  new ReflectionFile('');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}
