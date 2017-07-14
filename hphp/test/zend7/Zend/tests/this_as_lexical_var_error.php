<?php

class Foo {
    public function fn() {
        return function() use ($this) {};
    }
}

?>
