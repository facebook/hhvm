<?php

$x = tempnam("/tmp/", "xxxx");
var_dump(strpos($x, '//') === false);

$x = tempnam("/tmp//", "xxxx");
var_dump(strpos($x, '//') === false);

$x = tempnam("/tmp///", "xxxx");
var_dump(strpos($x, '//') === false);
