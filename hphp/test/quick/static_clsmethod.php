<?php
class C {
  public function className() {
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
    var_dump(isset($this));
    var_dump(static::className());
  }
  public function yar() {
    var_dump(isset($this));
    var_dump(static::className());
  }
}

class D extends C {
  public function className() {
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

function main() {
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
  echo "**************\n";
  static::foo();
}

main();

