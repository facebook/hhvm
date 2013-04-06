<?php
error_reporting(E_ALL);

trait HelloWorld {
   public function sayHello() {
     echo 'Hello World!';
   }
}

class MyClass {
   use HelloWorld { sayHello as private sayHelloWorld; }

   public function callPrivateAlias() {
      $this->sayHelloWorld();
   }
}

$o = new MyClass();
$o->sayHello();
$o->callPrivateAlias();
$o->sayHelloWorld();


?>