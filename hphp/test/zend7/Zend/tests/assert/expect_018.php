<?php
namespace Foo;

ini_set("zend.assertions", 0);
var_dump(\assert(false));
var_dump(\assert(true));
var_dump(assert(false));
var_dump(assert(true));
ini_set("zend.assertions", 1);
var_dump(\assert(false));
var_dump(\assert(true));
var_dump(assert(false));
var_dump(assert(true));
?>
