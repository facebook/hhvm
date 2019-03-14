<?hh
class C {
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::class);
  }
  public function foo() {
    var_dump(isset($this));
    var_dump(static::class);
  }
  public static function bar() {
    var_dump(static::class);
  }
  public function yar() {
    var_dump(isset($this));
    var_dump(static::class);
  }
}

class D extends C {
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::class);
  }
  public function yar() {
    var_dump(isset($this));
    var_dump(static::class);
    C::yar();
  }
}

echo "**************\n";
$c = new C;
$d = new D;
echo "**************\n";
$c->foo();
$d->foo();
echo "**************\n";
$c->bar();
$d->bar();
echo "**************\n";
C::foo();
D::bar();
echo "**************\n";
$d->yar();
D::yar();
