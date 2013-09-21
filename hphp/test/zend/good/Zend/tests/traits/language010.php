<?php
error_reporting(E_ALL);

trait Hello {
   public function hello() {
     echo 'Hello';
   }
}

trait World {
   public function world() {
     echo ' World!';
   }
}


class MyClass {
   use Hello, World { hello as world; }
}

$o = new MyClass();
$o->hello();
$o->world();

?>