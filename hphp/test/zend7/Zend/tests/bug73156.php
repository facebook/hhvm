<?php
class A {
    public function __call($name, $args) {
        eval('$args = array(); var_dump(debug_backtrace());');
    }
}

$a = new A();

$a->test("test");
?>
