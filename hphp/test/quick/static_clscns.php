<?hh
class C {
  const CLASSNAME = 'C';
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::CLASSNAME);
  }
  public function foo() :mixed{
    var_dump(isset($this));
    var_dump(static::CLASSNAME);
  }
  public static function bar() :mixed{
    var_dump(static::CLASSNAME);
  }
  public function yar() :mixed{
    var_dump(isset($this));
    var_dump(static::CLASSNAME);
  }
}

class D extends C {
  const CLASSNAME = 'D';
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::CLASSNAME);
  }
  public function yar() :mixed{
    var_dump(isset($this));
    var_dump(static::CLASSNAME);
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
