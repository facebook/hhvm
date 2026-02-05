<?hh

class A {
}

 <<__EntryPoint>>
function main_1261() :mixed{
$a = new A();
 $a->b = 'test';
 $a->a = $a->b;
 var_dump($a);
}
