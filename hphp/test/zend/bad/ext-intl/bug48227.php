<?php

$x = new NumberFormatter('en_US', NumberFormatter::DECIMAL);
var_dump($x->format(''));
var_dump($x->format(1));
var_dump($x->format(NULL));
var_dump($x->format($x));

?>