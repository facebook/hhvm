<?hh

class C {
  const type A = self::B;
  const type B = self::A;
}

class D<reify T>{}

function f(): D<C::A> {
  return new D<int>();
}

<<__EntryPoint>> function main() :mixed{
  var_dump(f());
}
