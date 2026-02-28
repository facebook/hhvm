<?hh

class A {
 public $a = null;
 }
class B extends A {
 public function foo() :mixed{
 var_dump($this->a);
}
 }
 class C extends B {
 public $a = 'test';
}

 <<__EntryPoint>>
function main_680() :mixed{
$obj = new C();
 $obj->foo();
}
