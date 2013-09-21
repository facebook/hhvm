<?php

$s = new SplObjectStorage();

var_dump($s->attach(true));
var_dump($s->attach(new stdClass, true, true));

?>