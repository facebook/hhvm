<?php

class bar {
 function baz() {
 yield 5;
 }
 }
$x = new ReflectionClass('bar');
var_dump(count($x->getMethods()));
