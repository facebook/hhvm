<?hh
class A {
  public $arr;
}

<<__EntryPoint>>
function main() :mixed{
  $obj = new A;
  $obj->arr = vec['test'];
  var_dump($obj->arr);
  unset($obj->arr);
  try {
    var_dump($obj->arr);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
}
