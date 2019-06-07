<?hh

class A {
 function _test() {
 print 'ok';
}
 function __call($name, $args) {
 $name = '_'.$name;
 $this->$name();
}
 }

<<__EntryPoint>>
function main_1208() {
$obj = new A();
 $obj->test();
}
