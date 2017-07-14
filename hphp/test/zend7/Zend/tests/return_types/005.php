<?php
class foo {}

class qux {
    public function foo() : foo {
        return $this;
    }
}

$qux = new qux();
$qux->foo();

