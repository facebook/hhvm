<?php

class foo {
    public $x = 1;

    public function getX() {
        return $this->x;
    }
    public function setX($val) {
        $this->x = $val;
        return $this;
    }
}
<<__EntryPoint>> function main() {
$X = (new foo)->setX(10)->getX();
var_dump($X); // int(10)
}
