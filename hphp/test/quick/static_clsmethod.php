<?hh
class C {
  public static function className() {
    return 'C';
  }
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::className());
  }
  public function foo() {
    var_dump(isset($this));
    var_dump(static::className());
  }
  public static function bar() {
    var_dump(static::className());
  }
  public function yar() {
    var_dump(isset($this));
    var_dump(static::className());
  }
}

class D extends C {
  public static function className() {
    return 'D';
  }
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::className());
  }
  public function yar() {
    var_dump(isset($this));
    var_dump(static::className());
    C::yar();
  }
}

<<__EntryPoint>> function main(): void {
  $c = new C;
  $d = new D;
  echo "**************\n";
  $c->foo();
  $d->foo();
  echo "**************\n";
  C::bar();
  D::bar();
  echo "**************\n";
  $c->yar();
  $d->yar();
  echo "**************\n";
  static::foo();
}
