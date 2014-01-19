<?php

function foo($a = null) {
}
$func = new ReflectionFunction('foo');
$params = $func->getParameters();
var_dump($params[0]->isDefaultValueAvailable());
