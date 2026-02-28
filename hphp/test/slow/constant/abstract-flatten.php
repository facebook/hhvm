<?hh

<<file: __EnableUnstableFeatures('class_const_default')>>

enum E: string as string {
  E1 = 'e1';
  E2 = 'e2';
  E3 = 'e3';
  E4 = 'e4';
}

interface I {
  abstract const FOO1 = E::E1;
  abstract const FOO2 = E::E2;
  abstract const FOO3 = E::E3;
}

trait T implements I {
}

class C1 {
  use T;
}

class C2 {
  use T;
  const FOO4 = E::E4;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(C1::FOO1);
  var_dump(C1::FOO2);
  var_dump(C1::FOO3);

  var_dump(C2::FOO1);
  var_dump(C2::FOO2);
  var_dump(C2::FOO3);
  var_dump(C2::FOO4);
}
