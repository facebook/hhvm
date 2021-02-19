<?hh

interface I1 {
  const FOO = "tinterface";
}

trait T implements I1 {}

interface I {
  const FOO = "interface";
}

class A implements I {
  use T;
}
