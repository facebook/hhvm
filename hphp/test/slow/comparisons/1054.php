<?php

$x = (object)null;
var_dump ($x == 1 && 1 == $x);
var_dump ($x == 1.0 && 1.0 == $x);
var_dump ($x > 0);
var_dump ($x >= 1);
var_dump ($x < 5);
var_dump ($x <= 1);
