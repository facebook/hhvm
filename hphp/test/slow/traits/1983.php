<?hh

class Base {
  public function sayHello() :mixed{
    echo 'Hello ';
  }
}
class MyHelloWorld extends Base {
  use SayWorld;
}
trait SayWorld {
  public function sayHello() :mixed{
    parent::sayHello();
    echo 'World!';
  }
}
<<__EntryPoint>> function main(): void {
$o = new MyHelloWorld();
$o->sayHello();
}
