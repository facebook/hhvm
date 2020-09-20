<?hh

class A {
 public function getA() {
 return $this;
}
 public function test() {
 var_dump('test');
}
}
 class B {
 public function getA() {
}
 public function test(){
}
}

<<__EntryPoint>>
function main_1271() {
$obj = new A();
 $obj->getA()->test();
}
