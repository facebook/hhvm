<?hh

class A {
  const type B = self::C;
  const type C = self::B;
}

type T = A::B;

<<__EntryPoint>> function main() {
  var_dump(\HH\type_structure('T'));
}
