<?php

$b = 'b';

$a = array(&$b);
var_dump(array_values($a));
