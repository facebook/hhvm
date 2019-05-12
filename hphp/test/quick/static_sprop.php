<?hh
class C {
  static $cls = 'C';
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::$cls);
  }
  public function foo() {
    var_dump(isset($this));
    var_dump(static::$cls);
  }
  public static function bar() {
    var_dump(static::$cls);
  }
  public function yar() {
    var_dump(isset($this));
    var_dump(static::$cls);
  }
}

class D extends C {
  static $cls = 'D';
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::$cls);
  }
  public function yar() {
    var_dump(isset($this));
    var_dump(static::$cls);
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
}
