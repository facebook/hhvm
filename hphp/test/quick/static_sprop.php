<?hh
class C {
  public static $cls = 'C';
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::$cls);
  }
  public function foo() :mixed{
    var_dump(isset($this));
    var_dump(static::$cls);
  }
  public static function bar() :mixed{
    var_dump(static::$cls);
  }
  public function yar() :mixed{
    var_dump(isset($this));
    var_dump(static::$cls);
  }
}

class D extends C {
  public static $cls = 'D';
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::$cls);
  }
  public function yar() :mixed{
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
