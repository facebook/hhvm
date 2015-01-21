<?php

$fp = fopen('php://memory', 'r');
fclose($fp);

var_dump(get_resource_type($fp));
