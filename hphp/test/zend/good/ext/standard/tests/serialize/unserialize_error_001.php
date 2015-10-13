<?php
class foo {
        public $x = "bar";
}
$z = array(new foo(), 2, "3");
$s = serialize($z);

var_dump(unserialize($s, ["allowed_classes" => null]));
var_dump(unserialize($s, ["allowed_classes" => 0]));
var_dump(unserialize($s, ["allowed_classes" => 1]));
