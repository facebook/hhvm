<?php

class Foo {}

<<__EntryPoint>>
function main_reflection_function_get_closure_this() {
$unbound = function () {};
$bound = $unbound->bindTo(new Foo);

var_dump((new ReflectionFunction($unbound))->getClosureThis());
var_dump((new ReflectionFunction($bound))->getClosureThis());
}
