<?hh

class A {
}

 <<__EntryPoint>>
function main_751() {
$obj = new A();
 $obj->test = 'test';
 var_dump($obj->test);
}
