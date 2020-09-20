<?hh

type T = null;

class C {
  const type T1 = null;
  const type T2 = dict<string, null>;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(T::class));
var_dump(type_structure(C::class, 'T1'));
var_dump(type_structure(C::class, 'T2'));
}
