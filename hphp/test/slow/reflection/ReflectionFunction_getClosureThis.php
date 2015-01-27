<?php

class Foo {}
$unbound = function () {};
$bound = $unbound->bindTo(new Foo);

var_dump((new ReflectionFunction($unbound))->getClosureThis());
var_dump((new ReflectionFunction($bound))->getClosureThis());
