<?hh
<<__EntryPoint>> function main(): void {
try {
  new ReflectionFile('');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}
}
