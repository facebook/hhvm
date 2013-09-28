<?php

$a = array(1,2);
unset($a[1.5]);
var_dump($a);
