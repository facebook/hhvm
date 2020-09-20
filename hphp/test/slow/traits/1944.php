<?hh

trait HelloWorld {
  public function sayHello() {
    echo "Hello World!\n";
  }
}
class TheWorldIsNotEnough {
  use HelloWorld;
  public function sayHello() {
    echo "Hello Universe!\n";
  }
}
<<__EntryPoint>> function main(): void {
$o = new TheWorldIsNotEnough();
$o->sayHello();
}
