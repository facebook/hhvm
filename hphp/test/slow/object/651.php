<?hh

interface I {
 public function test($a):mixed;
}
class A implements I {
 public function test($a) :mixed{
 print $a;
}
}

<<__EntryPoint>>
function main_651() :mixed{
$obj = new A();
 var_dump($obj is I);
 $obj->test('cool');
}
