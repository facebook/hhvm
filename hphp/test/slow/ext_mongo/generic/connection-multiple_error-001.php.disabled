<?php
require_once __DIR__."/../utils/server.inc";

/* We forget to specify the replicaset name */
$d = @new Mongo("mongodb://foofas:234,foofas:5345");
$c = $d->phpunit->test1;
var_dump( $c->findOne() );
?>