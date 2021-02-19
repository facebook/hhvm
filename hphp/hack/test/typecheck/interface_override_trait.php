<?hh

trait T {
  const FOO1 = "trait";
}

interface I {
  const FOO1 = "interface";
}

class A implements I {
  use T;
}
