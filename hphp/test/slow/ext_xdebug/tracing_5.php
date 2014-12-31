<?php

function start_trace() {
  xdebug_start_trace('/tmp/trace.%p', XDEBUG_TRACE_HTML);
}

start_trace();
require(__DIR__ . '/tracing.inc');
$file = xdebug_stop_trace();
var_dump(file_get_contents($file));
