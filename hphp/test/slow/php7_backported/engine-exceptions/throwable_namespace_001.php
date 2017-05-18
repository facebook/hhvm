<?php

namespace Foo;
$y = 'Throwable';

$x = new \Exception();
var_dump($x instanceof \Throwable);
var_dump($x instanceof Throwable);
var_dump($x instanceof $y);
