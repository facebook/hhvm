<?php

namespace foo;

$a = function () {};
$r = new \ReflectionFunction($a);
var_dump($r->inNamespace());
var_dump($r->getNamespaceName());
var_dump($r->getName());
