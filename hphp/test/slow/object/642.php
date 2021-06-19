<?hh
class A {
 public $test = 'ok';
}
<<__EntryPoint>> function main(): void {
  $obj = new A();
  var_dump($obj);
  var_dump((bool)$obj);
  try {
    var_dump((int)$obj);
  } catch (TypecastException $e) {
    var_dump($e->getMessage());
  }
}
