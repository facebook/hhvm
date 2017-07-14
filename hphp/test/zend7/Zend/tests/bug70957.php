<?php

abstract class Foo
{
    function bar($a = self::class) {}
}

trait T {
    public function bar() {
    }
}

class B extends Foo
{
    use T;
}
?>
