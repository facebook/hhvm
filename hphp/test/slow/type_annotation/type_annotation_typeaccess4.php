<?hh

class C {
  const type T = NonExistentClass;
  const type U = this::T;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(C::class, 'U'));
}
