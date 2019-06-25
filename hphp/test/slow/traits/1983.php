<?hh

class Base {
  public function sayHello() {
    echo 'Hello ';
  }
}
class MyHelloWorld extends Base {
  use SayWorld;
}
trait SayWorld {
  public function sayHello() {
    parent::sayHello();
    echo 'World!';
  }
}
<<__EntryPoint>> function main(): void {
$o = new MyHelloWorld();
$o->sayHello();
}
