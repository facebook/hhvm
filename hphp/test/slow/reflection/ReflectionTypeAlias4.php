<?hh

class NonExistentTypeAlias {
}


<<__EntryPoint>>
function main_reflection_type_alias4() {
try {
  $x = new ReflectionTypeAlias('NonExistentTypeAlias');
} catch (ReflectionException $e) {
  echo 'ReflectionException: ', $e->getMessage(), "\n";
}
}
