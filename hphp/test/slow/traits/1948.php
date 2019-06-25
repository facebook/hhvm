<?hh

trait SayWorld {
  public function sayHello() {
    echo 'Hello from trait!';
  }
}
class MyHelloWorld {
  public function sayHello() {
    echo 'Hello from class!';
  }
  use SayWorld;
}
<<__EntryPoint>> function main(): void {
$o = new MyHelloWorld();
$o->sayHello();
}
