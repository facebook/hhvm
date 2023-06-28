<?hh

class A {
  public $a = 3;
  public function __construct($a) {
    $this->a = $a + 1;
  }
}
class B extends A {
  public function __construct($a) {
  }
}
class C extends A {
  public function __construct($a) {
    parent::__construct($a);
  }
}

<<__EntryPoint>>
function main_645() :mixed{
$obj = new A(1);
 var_dump($obj->a);
$obj = new B(1);
 var_dump($obj->a);
$obj = new C(1);
 var_dump($obj->a);
unset($obj);
}
