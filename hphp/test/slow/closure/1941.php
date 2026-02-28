<?hh

class A {
  public function foo() :mixed{
    $values = vec[1, 2, 3];
    $values = array_map(function($p) {
      return $this->goo($p);
    }
, $values);
    var_dump($values);
  }
  public function bar() :mixed{
 return $this;
 }
  public function goo($p) :mixed{
 return $p;
 }
}

<<__EntryPoint>>
function main_1941() :mixed{
$obj = new A;
var_dump($obj->bar());
$obj->foo();
}
