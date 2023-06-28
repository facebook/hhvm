<?hh

class A {
}
class B {
  public $data;
  public function setData(?A $result = null) :mixed{
    $this->data = $result;
  }
}
function foo($obj) :mixed{
  $obj->data = new A;
  $a = $obj->data;
  var_dump($a);
  $obj->setData(null);
  $a = $obj->data;
  var_dump($a);
}

<<__EntryPoint>>
function main_696() :mixed{
foo(new B);
}
