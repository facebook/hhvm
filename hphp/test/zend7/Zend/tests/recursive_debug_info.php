<?php

class Test {
    public function __debugInfo() {
        return [$this];
    }
}

var_dump(new Test);

?>
