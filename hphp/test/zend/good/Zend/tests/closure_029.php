<?php

var_dump(function() { } instanceof closure);
var_dump(function(&$x) { } instanceof closure);
var_dump(@function(&$x) use ($y, $z) { } instanceof closure);

?>