<?hh

trait T {
  public function init() {
    parent::init();
  }
}
class A {
  public function init() {
    echo 'A::init';
  }
}
class B extends A {
  use T;
}
class C extends B {
  public function init() {
    parent::init();
  }
}

<<__EntryPoint>>
function main_2109() {
$obj = new C;
$obj->init();
}
