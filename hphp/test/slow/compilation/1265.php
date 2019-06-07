<?hh

class A {
 public $prop = 1;
}

 <<__EntryPoint>>
function main_1265() {
$a = new A();
 $a->prop++;
 var_dump($a->prop);
}
