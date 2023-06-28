<?hh

class A {
 public function getA() :mixed{
 return $this;
}
 public function test() :mixed{
 var_dump('test');
}
}
 class B {
 public function getA() :mixed{
}
 public function test():mixed{
}
}

<<__EntryPoint>>
function main_1271() :mixed{
$obj = new A();
 $obj->getA()->test();
}
