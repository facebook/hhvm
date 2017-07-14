<?php

class A {
    public static function test() {
        var_dump(static::class);
    }
}

class B extends A {
}

Closure::fromCallable(['A', 'test'])();
Closure::fromCallable(['B', 'test'])();

?>
