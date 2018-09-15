<?php
class foo {
    public $value = 42;

    public function &getValue() {
        return $this->value;
    }
}
// var_dump(strlen("haha"));


<<__EntryPoint>>
function main_disable_return_by_reference() {
$obj = new foo;
$myValue = &$obj->getValue();
$obj->value = 2;
echo $myValue;
}
