<?php
$x = new stdClass();
var_dump(var_export($x, true));
$x->herp = 'derp';
var_dump(var_export($x, true));
class Foo {
}
$y = new Foo();
var_dump(var_export($y, true));
