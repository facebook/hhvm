<?hh // strict

<<__ConsistentConstruct>>
abstract class X {
  abstract const type T as arraykey = arraykey;

  private this::T $priv;

  public function __construct(private this::T $val) {
    $this->priv = $val;
  }

  public static function test(this::T $t): void {
    $static = new static($t);
    $y = new Y(0);
  }
}

class Y extends X {
  const type T = int;

  public function __construct(this::T $val) {
    parent::__construct($val);
  }

  public static function test(this::T $t): void {
    $static = new static($t);
    // Since T cannot be overridden by sub-classes, we can pass in an int
    $static = new static(0);
    $self = new self(0);
    $y = new Y(0);
  }
}
