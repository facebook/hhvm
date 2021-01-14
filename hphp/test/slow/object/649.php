<?hh
class A {
  public $a = 2;
}
class B extends A {
  public $b = 3;
}
<<__EntryPoint>>
function main(): void {
  $obj = new A();
  var_dump($obj);
  try {
    var_dump($obj->b);
  } catch (UndefinedPropertyException $e) {
    var_dump($e->getMessage());
  }
  $obj = new B();
  var_dump($obj);
  var_dump($obj->b);
}
