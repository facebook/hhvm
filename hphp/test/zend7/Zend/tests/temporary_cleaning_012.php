<?php

class Foo {
    public static function test() {
        self::$property = new self;
    }
}

try {
    Foo::test();
} catch (Error $e) {
    echo $e->getMessage(), "\n";
}

?>
