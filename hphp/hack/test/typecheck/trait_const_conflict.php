<?hh

trait T {
  const FOO = "a";
}

trait T1 {
  const FOO = "b";
}

class A {
  use T, T1; // ok
}
