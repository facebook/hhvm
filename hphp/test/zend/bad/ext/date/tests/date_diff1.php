<?php
$start = new DateTime('2010-10-04 02:18:48 EDT');
$end   = new DateTime('2010-11-06 18:38:28 EDT');
$int = $start->diff($end);
var_dump($start);
var_dump($end);
var_dump($int);
?>