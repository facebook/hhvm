<?php

var_dump(preg_match('/a e i o u/', 'aeiou', $m));
var_dump($m);

var_dump(preg_match('/a e i o u/x', 'aeiou', $m));
var_dump($m);

var_dump(preg_match("/a e\ni\to\ru/x", 'aeiou', $m));
var_dump($m);

?>