<?hh
class C {
  public $cls = 'C';
  public function foo() {
    var_dump(isset($this));
    $obj = new static;
    var_dump($obj->cls);
  }
  public static function bar() {
    $obj = new static;
    var_dump($obj->cls);
  }
  public function yar() {
    var_dump(isset($this));
    $obj = new static;
    var_dump($obj->cls);
  }
}

class D extends C {
  public $cls = 'D';
  public function yar() {
    var_dump(isset($this));
    $obj = new static;
    var_dump($obj->cls);
    C::yar();
  }
}

<<__EntryPoint>> function main(): void {
  $c = new C;
  $d = new D;

  $c->foo();
  $d->foo();
  echo "**************\n";
  C::bar();
  D::bar();
  echo "**************\n";
  $c->yar();
  $d->yar();
}
