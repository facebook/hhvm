<?php

var_dump(array_fill(-2, -2, 'pear'));
var_dump(array_combine(array(1, 2), array(3)));
var_dump(array_combine(array(), array()));
var_dump(array_chunk(1));
var_dump(array_chunk(array()));
$a = array(1, 2);
var_dump(asort($a, 100000));
