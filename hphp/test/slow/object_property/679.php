<?hh

class A {
 public $a = 10;
 public function foo() :mixed{
 $this->a = 20;
}
 }
 class B extends A {
 public $a = 'test';
}

 <<__EntryPoint>>
function main_679() :mixed{
$obj = new B();
 $obj->foo();
 var_dump($obj->a);
}
