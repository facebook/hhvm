<?php
class foo {
        public $x = "bar";
}
$z = array(new foo(), 2, "3");
$s = serialize($z);

var_dump(unserialize($s));
var_dump(unserialize($s, ["allowed_classes" => false]));
var_dump(unserialize($s, ["allowed_classes" => true]));
var_dump(unserialize($s, ["allowed_classes" => ["bar"]]));
var_dump(unserialize($s, ["allowed_classes" => ["FOO"]]));
var_dump(unserialize($s, ["allowed_classes" => ["bar", "foO"]]));
