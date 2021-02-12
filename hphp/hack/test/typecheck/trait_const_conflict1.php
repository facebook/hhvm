<?hh

trait T {
  const FOO = "a";
}

interface I {
  const FOO = "b";
}

class A implements I { // will fatal at runtime
  use T;
}
