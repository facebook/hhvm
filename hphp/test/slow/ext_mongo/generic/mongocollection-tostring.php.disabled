<?php
require_once __DIR__."/../utils/server.inc";
$m = mongo_standalone();
$c = $m->phpunit->col;
echo "This is collection $c\n";
echo "This is collection ".$c->__toString()."\n";
$c = $m->selectCollection('phpunit', 'col2');
echo "This is collection $c\n";
echo "This is collection ".$c->__toString()."\n";
?>