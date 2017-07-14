<?php
class A {
    public function m(array $a = null) {}
}
class B extends A {
    public function m(array $a = []) {}
}
