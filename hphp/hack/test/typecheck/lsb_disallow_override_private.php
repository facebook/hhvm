<?hh // strict

class A {
  <<__LSB>> private static int $x = 0;
}

class B extends A {
  private static int $x = 1;
}
