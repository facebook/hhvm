<?php
error_reporting(E_ALL);

trait HelloWorld {
   public function sayHello() {
     echo 'Hello World!';
   }
}

class MyClass {
   use HelloWorld { sayHello as protected; }
}


$o = new MyClass;
$o->sayHello();

?>