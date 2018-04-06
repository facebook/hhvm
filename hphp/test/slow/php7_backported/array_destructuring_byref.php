<?php

$a = 123;

list(&$a, $b) = ['herp', 'derp'];
var_dump($a);
