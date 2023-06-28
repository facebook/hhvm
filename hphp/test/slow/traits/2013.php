<?hh

trait Hello {
   public function sayHello() :mixed{
     echo 'Hello ';
   }
}

trait World {
   public function sayWorld() :mixed{
     echo 'World!';
   }
}

trait HelloWorld {
   use Hello, World;
}

class MyHelloWorld {
   use HelloWorld;
}
<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);

$o = new MyHelloWorld();
$o->sayHello();
$o->sayWorld();
}
