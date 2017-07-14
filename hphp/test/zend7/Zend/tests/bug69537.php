<?php
class Foo {

    public function __debugInfo(){
        return ['' => 1];
    }
}

var_dump(new Foo);
?>
