<?hh

class A {
  <<__LSB>> protected static int $x = 0;
}

class B extends A {
  protected static int $x = 1;
}
