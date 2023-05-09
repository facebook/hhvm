<?hh

type MyAlias = shape('foo'=>E);

enum E: int {
  VAL = 42;
}

<<__EntryPoint>>
function main_entry(): void {
  var_dump((new ReflectionTypeAlias('MyAlias'))->getTypeStructure());
  var_dump(type_structure('MyAlias'));
}
