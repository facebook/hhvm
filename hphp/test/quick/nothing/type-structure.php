<?hh

type T = nothing;

class C {
  const type T1 = nothing;
  const type T2 = ?nothing;
  const type T3 = vec<nothing>;
  const type T4 = dict<string, nothing>;
}
<<__EntryPoint>> function main(): void {
var_dump(type_structure(T::class));
var_dump(type_structure(C::class, 'T1'));
var_dump(type_structure(C::class, 'T2'));
var_dump(type_structure(C::class, 'T3'));
var_dump(type_structure(C::class, 'T4'));
}
