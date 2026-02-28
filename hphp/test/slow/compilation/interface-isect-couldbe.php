<?hh

interface I1 {}
interface I2 {}
interface I3 {}

class C1 implements I2, I3 {}
class C2 implements I2, I3 {}
class C3 implements I1, I2 {}
class C4 implements I1, I3 {}

function bar($x) :mixed{
  if (!($x is I2)) return 123;
  if (!($x is I3)) return 456;
  if (!($x is I1)) return 789;
  return 0;
}

<<__EntryPoint>> function main() :mixed{
  var_dump(bar(new C1()));
}
