<?hh

trait SayWorld {
  public function sayHelloWorld() :mixed{
    self::sayHello();
    echo 'World!';
  }
}
class MyHelloWorld {
  use SayWorld;
  public function sayHello() :mixed{
    echo 'Hello ';
  }
}
<<__EntryPoint>> function main(): void {
$o = new MyHelloWorld();
$o->sayHelloWorld();
}
