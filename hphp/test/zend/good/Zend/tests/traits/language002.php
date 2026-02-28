<?hh

trait Hello {
   public function sayHello() :mixed{
     echo 'Hello ';
   }
}

trait World {
   public function sayWorld() :mixed{
     echo 'World';
   }
}

class MyHelloWorld {
   use Hello, World;
   public function sayExclamationMark() :mixed{
     echo '!';
   }
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$o = new MyHelloWorld();
$o->sayHello();
$o->sayWorld();
$o->sayExclamationMark();
}
