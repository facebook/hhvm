<?hh

interface I1 {
  const BAR = "green";
}

trait T1 implements I1 {
  const FOO = "blue";
}

interface I2 {
  const BAR = "purple";
}

trait T2 implements I2 {
  const FOO = "red";
}

class C {
  use T1, T2;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C::BAR);
  var_dump(C::FOO);
}
