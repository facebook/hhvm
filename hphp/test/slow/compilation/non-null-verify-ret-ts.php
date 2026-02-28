<?hh

class C {
  const type A = self::B;
  const type B = self::A;
}

class D<reify T>{}

function f(): D<C::A> {
  $x = new D<int>();
  if (__hhvm_intrinsics\launder_value(false)) $x = null;
  return $x;
}

<<__EntryPoint>> function main() :mixed{
  var_dump(f());
}
