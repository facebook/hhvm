<?php

var_dump(xdebug_code_coverage_started());

xdebug_start_code_coverage();
var_dump(xdebug_code_coverage_started());

var_dump(xdebug_get_code_coverage());
var_dump(xdebug_code_coverage_started());

xdebug_stop_code_coverage();
var_dump(xdebug_code_coverage_started());

xdebug_start_code_coverage();
var_dump(xdebug_get_code_coverage());
xdebug_stop_code_coverage(false);
xdebug_start_code_coverage();
var_dump(xdebug_get_code_coverage());
