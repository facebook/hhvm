<?php

$a = new stdClass;
$a->a = 'a';
var_dump((new ReflectionObject($a))->getProperty('a'));
var_dump((new ReflectionProperty($a, 'a')));
