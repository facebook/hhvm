<?hh

trait MY_TRAIT  {
  public function sayHello() :mixed{
    echo 'World!';
  }
}
class MY_CLASS {
  use MY_TRAIT;
}
<<__EntryPoint>> function main(): void {
$MY_OBJ = new MY_CLASS();
$MY_OBJ->sayHello();
}
