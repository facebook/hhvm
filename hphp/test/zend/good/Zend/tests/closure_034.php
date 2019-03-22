<?php

class C { public $a; }

$c = new C();
$c->a = function () use($c) {};
var_dump($c->a);

echo "===DONE===\n";
