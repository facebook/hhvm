<?hh

class C {
  const type T = this;
  const type U = NonExistentClass;
  const type V = C::T::U::T;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(C::class, 'V'));
}
