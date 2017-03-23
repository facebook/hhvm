<?php

$c = str_repeat('*', 769000);
var_dump(explode($c, $c, PHP_INT_MIN));
