<?hh

class A {
  const type B = self::C;
  const type C = self::B;
}

<<__EntryPoint>> function main() :mixed{
  var_dump(null as A::B);
}
