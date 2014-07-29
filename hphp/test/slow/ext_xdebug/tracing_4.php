<?php
xdebug_start_trace('/tmp/trace.%p');
require(__DIR__ . '/tracing.inc');
$file = xdebug_stop_trace();
xdebug_start_trace($file, XDEBUG_TRACE_APPEND | XDEBUG_TRACE_NAKED_FILENAME);
require(__DIR__ . '/tracing.inc');
xdebug_stop_trace();
var_dump(file_get_contents($file));
