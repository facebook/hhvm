<?php
class C {
  const CLASSNAME = 'C';
  public function __construct() {
    var_dump(isset($this));
    var_dump(static::CLASSNAME);
  }
  public function foo() {
    var_dump(isset($this));
    var_dump(static::CLASSNAME);
  }
  public static function bar() {
    var_dump(isset($this));
    var_dump(static::CLASSNAME);
  }
  public function yar() {
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
  public function yar() {
    var_dump(isset($this));
    var_dump(static::CLASSNAME);
    C::yar();
  }
}

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

