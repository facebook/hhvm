<?php

class Foo {
    public $array;

    public function __destruct() {
        var_dump(count($this->array[0]));
        var_dump($this->array[0]);
    }
}

$array = [[new Foo]];
$array[0][0]->array =& $array;
unset($array[0][0]);

?>