<?php
$refl = new ReflectionClass("Memcached");
$constants = $refl->getConstants();
var_dump($constants["RES_SERVER_MARKED_DEAD"]);
var_dump($constants["OPT_SORT_HOSTS"]);
var_dump($constants["OPT_VERIFY_KEY"]);
