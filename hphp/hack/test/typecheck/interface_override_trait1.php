<?hh

trait T1 {
  const FOO1 = "trait";
}

trait T {
  use T1;
}

interface I {
  const FOO1 = "interface";
}

class A implements I {
  use T;
}
