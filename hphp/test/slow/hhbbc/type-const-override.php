<?hh

interface I1 {
  abstract const type T1 = shape(...);
  function foo():mixed;
}

interface I2 extends I1 {
  const type T1 = shape('A' => bool, 'B' => int);
}

interface I3 {
}

interface I4 {
}

final class B implements I3 {
}

final class D implements I4 {
}

final class C implements I3, I2, I4 {
  const type T2 = shape('A' => self::Type1);
  function foo() :mixed{ echo "foo()\n"; }
}

<<__EntryPoint>> function main() :mixed{
  $x = new C();
  $x->foo();
}
