<?hh

class A {
 public $a = 10;
 public function foo() {
 $this->a = 20;
}
 }
 class B extends A {
 public $a = 'test';
}

 <<__EntryPoint>>
function main_679() {
$obj = new B();
 $obj->foo();
 var_dump($obj->a);
}
