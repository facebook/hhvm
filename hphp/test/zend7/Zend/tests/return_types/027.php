<?php

class A {}
class B extends A {}

$c = function(parent $x): parent { return $x; };
var_dump($c->bindTo(null, 'B')(new A));

