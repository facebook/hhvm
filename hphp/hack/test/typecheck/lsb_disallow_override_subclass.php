<?hh // strict

class A {
  <<__LSB>> private static int $x = 0;
}

class B extends A {
}

class C extends B {
  private static int $x = 1;
}
