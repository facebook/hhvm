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
    parent::__construct($val);
  }

  public static function test(this::T $t): void {
    parent::__construct($t);

    /* This should be an error, because parent */
    parent::__construct('');
  }
}
