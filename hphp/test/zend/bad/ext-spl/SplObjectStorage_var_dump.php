<?php
$o = new SplObjectStorage();

$o[new StdClass] = $o;

var_dump($o);