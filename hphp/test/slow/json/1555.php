<?php

class Foo {
 }
$foo = new Foo();
 $foo->foo = $foo;
var_dump(json_encode($foo));
