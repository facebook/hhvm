<?php
// Source php weakref extension
class A {
    private $wr = null;
    public function __construct() {
        $this->wr = new WeakRef($this);
    }
    public function __destruct() {
        unset($this->wr);
    }
}
$a = new A;
unset($a);
