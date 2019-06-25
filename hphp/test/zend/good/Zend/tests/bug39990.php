<?hh
class Foo {
  public function __get($name) {
    return array('Hello', 'World');
  }
}
<<__EntryPoint>> function main(): void {
$obj=new Foo();
foreach($obj->arr as $value)
  echo "$value\n";
}
