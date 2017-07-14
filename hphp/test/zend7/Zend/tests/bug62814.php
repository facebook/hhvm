<?php

class A {
    private function test() { }
}

class B extends A {
    protected function test() { }
}

class C extends B {
    private function test() { }
}

?>
