<?php
$dt = new DateTime('2009-10-11');

$df = $dt->diff(new DateTime('2009-10-13'));

var_dump($df,
	$df->y,
	$df->m,
	$df->d,
	$df->h,
	$df->i,
	$df->s,
	$df->invert,
	$df->days);

?>
==DONE==