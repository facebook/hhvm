<?php

$fp = fopen('test/nonexist.txt', 'r');
var_dump(pclose($fp));
