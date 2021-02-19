<?hh

trait T1 {
  const FOO1 = "trait";
}

class B1 {
  const FOO1 = "parent";
}

class A1 extends B1 {
  use T1;
}
