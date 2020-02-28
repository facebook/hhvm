<?hh

class A {
  public function foo() {
    $values = varray[1, 2, 3];
    $values = array_map(function($p) {
      return $this->goo($p);
    }
, $values);
    var_dump($values);
  }
  public function bar() {
 return $this;
 }
  public function goo($p) {
 return $p;
 }
}

<<__EntryPoint>>
function main_1941() {
$obj = new A;
var_dump($obj->bar());
$obj->foo();
}
