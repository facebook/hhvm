<?hh

type MyAlias = shape('foo'=>E);

$x = new ReflectionTypeAlias('MyAlias');
var_dump($x->getTypeStructure());

enum E: int {
  VAL = 42;
}
