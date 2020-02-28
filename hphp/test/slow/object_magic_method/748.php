<?hh

class A {
   public $a = varray[];
   function __set($name, $value) {
 $this->a[$name] = $value.'set';
}
   function __get($name) {
 return $this->a[$name].'get';
}
 }

 <<__EntryPoint>>
function main_748() {
$obj = new A();
 $obj->test = 'test';
 var_dump($obj->test);
}
