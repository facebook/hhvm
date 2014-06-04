<?php

$ar = array('a'=>0, 1, 2, 3);
$ar = new ArrayObject($ar);

unset($ar['a']);
unset($ar[12]);
unset($ar['c']);
$ar->offsetUnset('c');


$obj = new stdClass();
$obj->one = 1;
$obj->two = 2;
$ar = new ArrayObject($obj);

unset($ar['one']);
unset($ar[12]);
unset($ar['c']);
$ar->offsetUnset('c');
