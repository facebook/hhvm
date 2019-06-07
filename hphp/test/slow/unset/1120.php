<?hh

class A {
  public function foo() {
    unset($this);
    var_dump($this);
  }
}
function goo() {
  unset($this);
  var_dump($this);
}
$obj = new A;
$obj->foo();

goo();
unset($this);
var_dump($this);
