<?hh
class A {
  public $arr;
}

$obj = new A;
$obj->arr = varray['test'];
var_dump($obj->arr);
unset($obj->arr);
var_dump($obj->arr);
