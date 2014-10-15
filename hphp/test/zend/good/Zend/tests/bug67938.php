<?php

interface TestInterface {
    public function foo();
    public function bar(array $bar);
}

class Test implements TestInterface {
    public function foo(...$args) {
        echo __METHOD__, "\n";
    }
    public function bar(array $bar, ...$args) {
        echo __METHOD__, "\n";
    }
}

$obj = new Test;
$obj->foo();
$obj->bar([]);

?>
