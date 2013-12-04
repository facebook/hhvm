<?php
$d = new DateTime('@100000000000');
var_dump($d->format('Y-m-d H:i:s U'));
var_dump($d->getTimestamp());

$d->setTimestamp(100000000000);
var_dump($d->format('Y-m-d H:i:s U'));
var_dump($d->getTimestamp());

$i = new DateInterval('PT100000000000S');
var_dump($i->format('%s'));
?>