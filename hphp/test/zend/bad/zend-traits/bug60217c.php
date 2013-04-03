<?php

trait TBroken1 {
    public abstract function foo($a, $b = 0);
}

trait TBroken2 {
    public abstract function foo($a);
}

class CBroken {
    use TBroken1, TBroken2;

    public function foo($a) {
        echo 'FOO';
    }
}

$o = new CBroken;
$o->foo(1);
