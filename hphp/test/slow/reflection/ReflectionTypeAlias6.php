<?hh

type MyAlias = shape('foo'=>E);

enum E: int {
  VAL = 42;
}
<<__EntryPoint>>
function main_entry(): void {

  $x = new ReflectionTypeAlias('MyAlias');
  var_dump($x->getTypeStructure());
}
