<?hh

class C {
  const type A = self::B;
  const type B = self::A;
}

class D<reify T>{}

function f(D<C::A> $x) {
  var_dump($x);
}

<<__EntryPoint>> function main() {
  $d = new D<int>();
  f($d);
}
