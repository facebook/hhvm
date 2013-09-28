<?php

$a = new stdClass();
$a->{0} = 'X';
$a->{1} = 'Y';
var_dump(serialize($a));
var_dump($a->{0});
$b = unserialize(serialize($a));
var_dump(serialize($b));
var_dump($b->{0});