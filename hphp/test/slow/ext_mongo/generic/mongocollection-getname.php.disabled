<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone();

$c = $m->phpunit->col;
echo "Working with collection " . $c->getName() . ".\n";
$c = $m->phpunit->col->foo;
echo "Working with collection " . $c->getName() . ".\n";
$c = $m->phpunit->col->foo->bar;
echo "Working with collection " . $c->getName() . ".\n";
?>