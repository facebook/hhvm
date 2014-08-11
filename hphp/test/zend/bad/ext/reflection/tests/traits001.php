<?php
trait Foo {
    public function someMethod() { }
}

class Bar {
    use Foo;

    public function someOtherMethod() { }
}

$rFoo = new ReflectionClass('Foo');
$rBar = new ReflectionClass('Bar');

var_dump($rFoo->isTrait());
var_dump($rBar->isTrait());
echo $rFoo;
echo $rBar;
