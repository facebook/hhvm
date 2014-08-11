<?php

include "included4.inc";

$funcInfo = new ReflectionFunction('g');
var_dump($funcInfo->getFileName());

?>
