<?php
require_once __DIR__."/../../utils/server.inc";

$cstring = "$REPLICASET_SECONDARY:$REPLICASET_SECONDARY_PORT,$REPLICASET_PRIMARY:$REPLICASET_PRIMARY_PORT";

$a = new Mongo("mongodb://$cstring");
var_dump($a->__toString());

?>