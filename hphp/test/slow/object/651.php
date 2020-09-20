<?hh

interface I {
 public function test($a);
}
class A implements I {
 public function test($a) {
 print $a;
}
}

<<__EntryPoint>>
function main_651() {
$obj = new A();
 var_dump($obj is I);
 $obj->test('cool');
}
