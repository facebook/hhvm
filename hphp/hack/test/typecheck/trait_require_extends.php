<?hh // partial

trait T {
  require extends X;
}

class X {
  /* HH_FIXME[4336] */
  public static function foo(): X {

  }
}

class A extends X {
  /* HH_FIXME[4336] */
  public static function foo(): A {

  }
}

class B extends A {
  use T;
}
