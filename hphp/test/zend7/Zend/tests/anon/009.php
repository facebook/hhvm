<?php

trait Foo {
    public function someMethod() {
      return "bar";
    }
}

$anonClass = new class {
    use Foo;
};

var_dump($anonClass->someMethod());
