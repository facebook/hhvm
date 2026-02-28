<?hh

class A {
 public $prop = 1;
}
 class B {
 public $prop = 5;
}

 <<__EntryPoint>>
function main_1264() :mixed{
$a = 1;
 $a = new A();
 $a->prop++;
 var_dump($a->prop);
}
