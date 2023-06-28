<?hh

trait T1 {
  const FOO = "one";
}

trait T2 {
  use T1;
  const FOO = "two";
}

trait T3 {
  use T2;
  const FOO = "three";
}

trait T4 {
  const FOO = "four";
}

class C {
  use T3, T4;
}

class D {
  use T4, T3;
}

<<__EntryPoint>>
function main() :mixed{
  var_dump(D::FOO);
  var_dump(C::FOO);
  var_dump(T3::FOO);
  var_dump(T2::FOO);
}
