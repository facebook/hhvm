<?php

class Test {
    public function method() {
        eval('var_dump($this);');
    }
}

(new Test)->method();

?>
