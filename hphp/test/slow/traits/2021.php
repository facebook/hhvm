<?hh

trait HelloWorld {
   public function sayHello() :mixed{
     echo 'Hello World!';
   }
}

class TheWorldIsNotEnough {
   use HelloWorld;
   public function sayHello() :mixed{
     echo 'Hello Universe!';
   }
}

<<__EntryPoint>> function main(): void {
error_reporting(E_ALL);
$o = new TheWorldIsNotEnough();
$o->sayHello();
 // echos Hello Universe!
}
