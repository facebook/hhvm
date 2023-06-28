<?hh

class A {
 public $a = 1;
}
 class B {
 public $a = 2;
}

 <<__EntryPoint>>
function main_1169() :mixed{
$obj = 1;
 $obj = new A();
 var_dump($obj->a);
}
