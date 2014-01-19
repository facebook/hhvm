<?php

$a = array(1,2);
unset($a[true]);
var_dump($a);
