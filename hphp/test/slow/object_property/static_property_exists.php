<?php

class C {
    static protected $test = 'foo';
    public function test() {
        var_dump(property_exists($this, 'test'));
    }
}

$c = new C;
$c->test();
