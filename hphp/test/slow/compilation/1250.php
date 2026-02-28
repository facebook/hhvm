<?hh

class A {
 public $a = dict['t' => 't'];
}
 class B {
 public $a;
}

 <<__EntryPoint>>
function main_1250() :mixed{
$a = 1;
 $a = new A();
 $a->a['t'] = true;
 var_dump($a->a['t']);
}
