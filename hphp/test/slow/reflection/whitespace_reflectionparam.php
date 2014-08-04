<?php

function foo($bar) {}

$rf = new ReflectionFunction('foo');
var_dump((string) $rf->getParameters()[0]);
