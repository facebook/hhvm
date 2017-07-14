<?php
class C {
    public $ref;
    public $ary;
    public function __construct() {
        $this->ref = $this;
        $this->ary[] = 42;
    }
    public function __destruct() {
        global $ary;
        $ary[] = $this->ary[0];
    }
}

$c = new C;
unset($c);
gc_collect_cycles();

var_dump($ary);
?>
