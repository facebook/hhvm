<?hh
class A {
 public $test = 'ok';
}
<<__EntryPoint>> function main(): void {
  $obj = new A();
  var_dump($obj);
  var_dump((bool)$obj);
  var_dump((int)$obj);
  var_dump((array)$obj);
}
