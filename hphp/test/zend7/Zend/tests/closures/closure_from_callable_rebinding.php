<?php

class A {
    public function method() {
        var_dump($this);
    }
}

class B {
}

$fn = Closure::fromCallable([new A, 'method']);
$fn->call(new B);

?>
