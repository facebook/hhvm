<?php
error_reporting(E_ALL);

trait Hello {
   public function saySomething() {
     echo 'Hello';
   }
}

trait World {
   public function saySomething() {
     echo 'World';
   }
}
 
class MyHelloWorld {
   use Hello, World {
     Hello::saySomething insteadof World;
   }
}
 
$o = new MyHelloWorld();
$o->saySomething();
?>