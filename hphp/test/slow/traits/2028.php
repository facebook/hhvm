<?hh

abstract class Base {
  abstract function sayWorld();
}

trait Hello {
   public function sayHello() {
     echo 'Hello';
   }
   public function sayWorld() {
     echo ' World!';
   }
}

class MyHelloWorld extends Base {
    use Hello;
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$o = new MyHelloWorld();
$o->sayHello();
$o->sayWorld();
}
