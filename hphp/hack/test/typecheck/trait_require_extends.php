<?hh // partial

trait T {
  require extends X;
}

class X {
  public static function foo(): X {
    return new X();
  }
}

class A extends X {
  public static function foo(): A {
    return new A();
  }
}

class B extends A {
  use T;
}
