<?php

$a = array(1,2);
unset($a[false]);
var_dump($a);
