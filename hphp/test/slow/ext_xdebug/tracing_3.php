<?php
require(__DIR__ . '/tracing.inc');
$file = xdebug_stop_trace();
var_dump(file_get_contents($file));
