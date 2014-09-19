<?php

class Foo {
    public function test(...$args) {
        var_dump($args);
    }

    public static function test2(...$args) {
        var_dump($args);
    }
}

$foo = new Foo;
Foo::test2(1, 2, ...[3, 4], ...[], ...[5]);

?>
