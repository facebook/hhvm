<?hh

class A {
}

 <<__EntryPoint>>
function main_1261() :mixed{
$a = new A();
 $a->a = $a->b = 'test';
 var_dump($a);
}
