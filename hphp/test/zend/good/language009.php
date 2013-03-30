<?php
error_reporting(E_ALL);

trait A {
   public function foo() {
     echo 'a';
   }
}

trait B {
   public function foo() {
     echo 'b';
   }
}

trait C {
   public function foo() {
     echo 'c';
   }
}

class MyClass {
    use C, A, B {
		B::foo insteadof A, C; 
	}
}

$t = new MyClass;
$t->foo();

?>