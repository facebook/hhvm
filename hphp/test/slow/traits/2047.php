<?hh

class Base {
   public function sayHello() :mixed{
     echo 'Hello ';
   }
}

trait SayWorld {
   public function sayHello() :mixed{
     echo 'World!';
   }
}

class MyHelloWorld extends Base {
   use SayWorld;
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$o = new MyHelloWorld();
$o->sayHello();
}
