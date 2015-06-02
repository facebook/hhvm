<?hh // strict

class X {
  const type T as arraykey = string;

  private this::T $priv;

  public function __construct(private this::T $val) {
    $this->priv = $val;
  }

  public static function test(this::T $t): void {
    // Invalid because T can be overridden to accept only int,.
    $static = new static('');
  }
}
