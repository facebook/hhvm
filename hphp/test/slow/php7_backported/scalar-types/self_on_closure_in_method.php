<?php

class A {
    public function test() {
        return function() : self {
            return $this;
        };
    }
}

var_dump(((new A)->test())());

?>
