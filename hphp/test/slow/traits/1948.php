<?hh

trait SayWorld {
  public function sayHello() :mixed{
    echo 'Hello from trait!';
  }
}
class MyHelloWorld {
  public function sayHello() :mixed{
    echo 'Hello from class!';
  }
  use SayWorld;
}
<<__EntryPoint>> function main(): void {
$o = new MyHelloWorld();
$o->sayHello();
}
