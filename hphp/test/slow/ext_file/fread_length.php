<?php

$h = fopen(__FILE__, 'r');
var_dump(fread($h, 0));
