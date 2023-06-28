<?hh
class C {
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::class);
  }
  public function foo() :mixed{
    var_dump(isset($this));
    var_dump(static::class);
  }
  public static function bar() :mixed{
    var_dump(static::class);
  }
  public function yar() :mixed{
    var_dump(isset($this));
    var_dump(static::class);
  }
}

class D extends C {
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::class);
  }
  public function yar() :mixed{
    var_dump(isset($this));
    var_dump(static::class);
    C::yar();
  }
}
<<__EntryPoint>> function main(): void {
echo "**************\n";
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
