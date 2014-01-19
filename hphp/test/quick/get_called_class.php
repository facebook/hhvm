<?php
function f() {
  var_dump(get_called_class());
}

class C {
  public function __construct() {
    var_dump(isset($this));
    var_dump(get_called_class());
  }
  public function foo() {
    var_dump(isset($this));
    var_dump(get_called_class());
  }
  public static function bar() {
    var_dump(isset($this));
    var_dump(get_called_class());
  }
  public function yar() {
    var_dump(isset($this));
    var_dump(get_called_class());
  }
}

class D extends C {
  public function __construct() {
    var_dump(isset($this));
    var_dump(get_called_class());
  }
  public function yar() {
    var_dump(isset($this));
    var_dump(get_called_class());
    C::yar();
  }
}

var_dump(get_called_class());
f();
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

