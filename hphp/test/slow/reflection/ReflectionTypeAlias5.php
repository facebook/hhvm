<?hh

type MyAlias = shape('foo'=>E);

try {
  $x = new ReflectionTypeAlias('MyAlias');
  var_dump($x->getResolvedTypeStructure());
} catch (ReflectionException $ex) {
  echo 'ReflectionException: ', $ex->getMessage(), "\n";
}

enum E: int {
  VAL = 42;
}
