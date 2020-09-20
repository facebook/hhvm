<?hh
class A {
  public $arr;
}

<<__EntryPoint>>
function main() {
  $obj = new A;
  $obj->arr = varray['test'];
  var_dump($obj->arr);
  unset($obj->arr);
  var_dump($obj->arr);
}
