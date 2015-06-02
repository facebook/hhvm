<?hh // strict

<<__ConsistentConstruct>>
class X {
  const type T as arraykey = arraykey;

  private this::T $priv;

  public function __construct(private this::T $val) {
    $this->priv = $val;
  }

  public static function test(this::T $t): void {
    $static = new static($t);
    $self = new self('');
    $x = new X('');
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
    $x = new X('');
    $y = new Y(0);
  }
}
