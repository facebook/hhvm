<?php

trait T1 {
    public abstract function foo();
}

trait T2 {
    public abstract function foo();
}

class C {
    use T1, T2;

    public function foo() {
        echo "C::foo() works.\n";
    }
}

$o = new C;
$o->foo();
