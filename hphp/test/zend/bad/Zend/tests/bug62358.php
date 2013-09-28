<?php 

trait T {
    public function foo() {
        echo "from T";
    }
}

interface I {
    public function foo();
}

abstract class A implements I{
    use T;
}

class B extends A {
   public function foo($var) {
   } 
}
?>