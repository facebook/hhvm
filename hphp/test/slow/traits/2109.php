<?hh

trait T {
  public function init() :mixed{
    parent::init();
  }
}
class A {
  public function init() :mixed{
    echo 'A::init';
  }
}
class B extends A {
  use T;
}
class C extends B {
  public function init() :mixed{
    parent::init();
  }
}

<<__EntryPoint>>
function main_2109() :mixed{
$obj = new C;
$obj->init();
}
