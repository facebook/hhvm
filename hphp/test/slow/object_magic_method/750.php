<?hh

class A {
   public $a = varray[];
   function __set($name, $value) {
 $this->a[$name] = $value;
}
   function __get($name) {
 return $this->a[$name];
}
 }

 <<__EntryPoint>>
function main_750() {
$obj = new A();
 $obj->test = 'test';
 var_dump($obj->test);
}
