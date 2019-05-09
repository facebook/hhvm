<?hh // strict
<<__EntryPoint>> function main(): void {
try {
  new ReflectionFile(null);
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}
}
