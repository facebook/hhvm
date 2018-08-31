<?php

class MethodTest {
    public function __call($name, $arguments) {
        var_dump($name, implode(', ', $arguments));
    }
    public static function __callStatic($name, $arguments) {
        var_dump($name, implode(', ', $arguments));
    }
}

<<__EntryPoint>>
function main_1900() {
$obj = new MethodTest;
$obj->runTest('in object context');
MethodTest::runTest('in static context');
}
