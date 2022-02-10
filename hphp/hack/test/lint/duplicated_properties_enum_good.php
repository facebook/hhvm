<?hh

enum E : string as string {
  FOO = 'foo';
}

abstract class A {
  protected E $x = E::FOO;
}

abstract class B extends A {}

trait T0 {
  require extends A;
}

trait T1 {
  require extends A;
}

class C extends B {
  use T0;
  use T1;
}
