<?hh

class A {
  private $prop = 'test';

  function __get($name) {
    return $this->$name;
  }
}


<<__EntryPoint>>
function main_678() {
$obj = new A();
var_dump($obj->prop);
}
