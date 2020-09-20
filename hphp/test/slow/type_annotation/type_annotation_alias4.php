<?hh

newtype MyAlias = shape(
  'foo'=>NonExistentClass
);

class C {
  const type T = MyAlias;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(C::class, 'T'));
}
