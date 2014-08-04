<?php

function herp($derp) {}
function foo($bar, $baz) {}

var_dump((string) (new ReflectionFunction('herp')));
var_dump((string) (new ReflectionFunction('foo')));
