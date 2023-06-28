<?hh

trait HelloWorld {
   public function sayHello() :mixed{
     echo 'Hello World!';
   }
}

trait HelloWorld2 {
   public function sayHello() :mixed{
     echo 'Hello World2!';
   }
}


class TheWorldIsNotEnough {
   use HelloWorld;
   use HelloWorld2;
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
