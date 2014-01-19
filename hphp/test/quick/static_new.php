<?php
class C {
  public $cls = 'C';
  public function foo() {
    var_dump(isset($this));
    $obj = new static;
    var_dump($obj->cls);
  }
  public static function bar() {
    var_dump(isset($this));
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

function main() {
  $c = new C;
  $d = new D;

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
}

main();
