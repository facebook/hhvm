<?hh // strict

class X {
  const type T as arraykey = arraykey;

  private this::T $priv;

  public function __construct(private this::T $val) {
    $this->priv = $val;
  }
}

class Y extends X {
  const type T = int;

  public function __construct(this::T $val) {
    // Inside a constructor parent::__construct type constants are assumed to
    // compatible with this::T
    parent::__construct($val);
  }

  public static function test(this::T $t): void {
    // However in other context it is not compatible with this::T
    $parent = parent::__construct($t);
  }
}
