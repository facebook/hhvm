<?php
class foo {
    public $value = 42;

    public function &getValue() {
        return $this->value;
    }
}

$obj = new foo;
$myValue = &$obj->getValue();
$obj->value = 2;
echo $myValue;
// var_dump(strlen("haha"));
