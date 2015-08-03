<?hh

class NonExistentTypeAlias {
}

try {
  $x = new ReflectionTypeAlias('NonExistentTypeAlias');
} catch (ReflectionException $e) {
  echo 'ReflectionException: ', $e->getMessage(), "\n";
}
