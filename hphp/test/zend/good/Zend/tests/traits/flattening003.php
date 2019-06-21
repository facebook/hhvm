<?hh

trait Hello {
   public function sayHello() {
     echo 'Hello ';
   }
}

trait World {
   public function sayWorld() {
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
