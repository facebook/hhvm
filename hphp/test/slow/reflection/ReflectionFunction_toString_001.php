<?php

function foo($bar, $baz = 123) {}

echo (string)(new ReflectionFunction('foo'));
