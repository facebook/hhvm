<?hh

interface I1 {
  abstract const type T1;
}

interface I2 extends I1 {
  const type T1 = int;
}

interface I3 extends I2 {
  const type T1 = int;
}

abstract class C implements I3 {}

<<__EntryPoint>> function main() :mixed{
  var_dump(C::class);
}
