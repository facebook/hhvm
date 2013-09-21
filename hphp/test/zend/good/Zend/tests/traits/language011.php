<?php
error_reporting(E_ALL);

trait Hello {
   public function sayHello() {
     echo 'Hello';
   }
}

trait World {
   public function sayHello() {
     echo ' World!';
   }
}


class MyClass {
   use Hello, World { sayHello as sayWorld; }
}

$o = new MyClass();
$o->sayHello();
$o->sayWorld();

?>