<?php

class Foo {
 }

<<__EntryPoint>>
function main_1555() {
$foo = new Foo();
 $foo->foo = $foo;
var_dump(json_encode($foo));
}
