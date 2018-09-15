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

<<__EntryPoint>>
function main_weakref_destroy_in_dtor() {
$a = new A;
unset($a);
echo "Done\n";
}
