<?php
require_once __DIR__."/../utils/server.inc";

/* Two random names */
$d = @new Mongo("mongodb://foofas:234,foofas:5345/demo?replicaSet=seta");
$c = $d->phpunit->test1;
var_dump( $c->findOne() );
?>