<?php

// implicitly weak mode code

function to_int(int &$x) {}
function to_float(float &$x) {}
function to_string(string &$x) {}
function to_bool(bool &$x) {}

$x = 1.0;
var_dump($x);
to_int($x); // because $x is by-reference, the weak type hint converts it
var_dump($x);
to_float($x);
var_dump($x);
to_string($x);
var_dump($x);
to_bool($x);
var_dump($x);
?>
