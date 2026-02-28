<?hh
<<__EntryPoint>> function main(): void {
try {
  new ReflectionFile('NoSuchFile');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}
}
