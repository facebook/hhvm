<?php

class Test {
    public function method() {
        return function($this) {};
    }
}

(new Test)->method()(new stdClass);

?>
