<?hh

class A {
}
class B {
  public $data;
  public function setData(A $result = null) {
    $this->data = $result;
  }
}
function foo($obj) {
  $obj->data = new A;
  $a = $obj->data;
  var_dump($a);
  $obj->setData(null);
  $a = $obj->data;
  var_dump($a);
}

<<__EntryPoint>>
function main_696() {
foo(new B);
}
