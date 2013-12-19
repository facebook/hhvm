<?php
$t = new DateTime('2010-07-06 18:38:28 EDT'); 
$ts = $t->format('U');
var_dump($ts);
$t->setTimestamp($ts);
var_dump($t);
?>