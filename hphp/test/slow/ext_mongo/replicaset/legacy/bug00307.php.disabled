<?php
require_once __DIR__."/../../utils/server.inc";
$m = mongo();


$d = $m->phpunit->bug307;
var_dump($d->findOne());

$hosts = $m->getHosts();
$host  = current($hosts);
var_dump($host);
?>