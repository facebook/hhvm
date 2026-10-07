<?hh

class X {
  const type T = string;

  private this::T $priv;

  public function __construct(private this::T $val) {
    $this->priv = $val;
  }

  <<__NeedsConcrete>>
  public static function test(this::T $t): void {
    // Valid because T cannot be overridden
    $static = new static('');
  }
}
