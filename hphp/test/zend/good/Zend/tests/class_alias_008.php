<?php

abstract class foo { }

class_alias('foo', "\0");

$a = "\0";

new $a;

?>