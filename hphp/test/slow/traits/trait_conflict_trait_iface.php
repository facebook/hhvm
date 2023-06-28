<?hh

interface I1 {
  const BAR = "1bar";
}

trait T1 implements I1 {
  const FOO = "1foo";
}

interface I2 {
  const FOO = "2foo";
}

trait T2 implements I2 {
  const BAR = "2bar";
}

class C {
  use T1, T2;
}

class D {
  use T2, T1;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C::BAR);
  var_dump(C::FOO);
  var_dump(D::BAR);
  var_dump(D::FOO);
}
