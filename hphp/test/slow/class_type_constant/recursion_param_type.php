<?hh

class C {
  const type A = self::B;
  const type B = self::A;
}

class D<reify T>{}

function f(D<C::A> $x) :mixed{
  var_dump($x);
}

<<__EntryPoint>> function main() :mixed{
  $d = new D<int>();
  f($d);
}
