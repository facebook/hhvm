<?php
xdebug_start_code_coverage();
include "coverage.inc";
$cc = xdebug_get_code_coverage();
xdebug_stop_code_coverage();
var_dump($cc);
