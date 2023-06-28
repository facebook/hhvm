<?hh


<<__EntryPoint>>
function main_reflection_type_alias2() :mixed{
try {
  $x = new ReflectionTypeAlias('NoSuchTypeAlias');
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}
}
