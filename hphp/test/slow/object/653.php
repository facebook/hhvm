<?hh

class A {
 public $a;
}

 <<__EntryPoint>>
function main_653() :mixed{
$obj1 = new A();
 $obj2 = new A();
 $obj1->a = $obj2;
 $obj2->a = $obj1;
var_dump($obj1);
}
