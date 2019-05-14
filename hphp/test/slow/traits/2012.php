<?php

trait TestTrait {
    public static function test() {
        return 'Test';
    }
}

class A {
    use TestTrait;
}
<<__EntryPoint>> function main() {
$class = "A";
echo $class::test();
}
