<?php

class foo {
}

var_dump(class_exists());
var_dump(class_exists("qwerty"));
var_dump(class_exists(""));
var_dump(class_exists(array()));
var_dump(class_exists("test", false));
var_dump(class_exists("foo", false));
var_dump(class_exists("foo"));
var_dump(class_exists("stdClass", false));
var_dump(class_exists("stdClass"));

echo "Done\n";
?>