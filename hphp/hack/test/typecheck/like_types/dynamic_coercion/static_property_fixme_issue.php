<?hh // strict

/* This is a general problem with FIXME on property initializers. */
class C {
  /* HH_FIXME[4110] */
  public static (function (int): void) $f = 3;
}

function expect_string(string $s): void {}

class D extends C {
  public function __construct(): void {
    /* HH_FIXME[4110] */
    self::$f = ($s) ==> { expect_string($s); };
  }

  public function break(dynamic $d): void {
    self::$f($d); // this would be an error because the lambda parameter type is not enforceable

    self::$f("3"); // but so would this. The FIXME breaks both.
  }
}
