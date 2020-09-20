<?hh

class C {
  const type T1 = darray<string, ?int>;
  const type T2 = varray<darray<string, varray<C>>>;
  const type T3 = dict<string, ?int>;
  const type T4 = vec<darray<string, varray<C>>>;
}

<<__EntryPoint>>
function main(): void {
  var_dump(type_structure(C::class, 'T1'));
  var_dump(type_structure(C::class, 'T2'));
  var_dump(type_structure(C::class, 'T3'));
  var_dump(type_structure(C::class, 'T4'));
}
