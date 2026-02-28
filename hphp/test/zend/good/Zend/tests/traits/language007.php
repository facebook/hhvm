<?hh

abstract class Base {
  abstract function sayWorld():mixed;
}

trait Hello {
   public function sayHello() :mixed{
     echo 'Hello';
   }
   public function sayWorld() :mixed{
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
