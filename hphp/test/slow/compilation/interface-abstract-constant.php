<?hh

interface I {
  abstract const int C;
}

abstract class C1 implements I {}
abstract class C2 implements I {}
abstract class C3 implements I {}

class C4 extends C3 {
  const int C = 123;
}

function bar(I $x) :mixed{
  var_dump($x::C);
}

<<__EntryPoint>> function main() :mixed{
  bar(new C4());
}
