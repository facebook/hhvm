<?php

function test($className) {
$x = new ReflectionClass($className);
return $x->newInstance()->loadAll();
 }
