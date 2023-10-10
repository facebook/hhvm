<?hh

class C {
  const type T = NonExistentClass;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(C::class, 'T'));
}
