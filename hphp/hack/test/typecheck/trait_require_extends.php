<?hh

trait T {
  require extends X;
}

class X {
  public static function foo(): X {
    //UNSAFE
  }
}

class A extends X {
  public static function foo(): A {
    //UNSAFE
  }
}

class B extends A {
  use T;
}
