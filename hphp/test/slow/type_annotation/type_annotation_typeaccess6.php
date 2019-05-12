<?hh

class C {
  const type T = C::TClass::T;
  const type TClass = D;
}

class D {
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(C::class, 'T'));
}
