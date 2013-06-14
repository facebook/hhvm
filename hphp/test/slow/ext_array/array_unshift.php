<?php

$q = array("orange", "banana");
array_unshift($q, "apple", "raspberry");
var_dump($q);

$q = array(0 => "orange", 1 => "banana", "a" => "dummy");
unset($q['a']);
array_unshift($q, "apple", "raspberry");
var_dump($q);
