<?
require('tracing_basic.inc');

// Default
var_dump(xdebug_start_trace());
var_dump(xdebug_get_tracefile_name());
var_dump(xdebug_stop_trace());

// Naked
var_dump(xdebug_start_trace(null, XDEBUG_TRACE_NAKED_FILENAME));
var_dump(xdebug_stop_trace());

// Non default
var_dump(xdebug_start_trace("/tmp/trace"));
var_dump(xdebug_stop_trace());

// crc32
var_dump(xdebug_start_trace("/tmp/trace-%c"));
var_dump(xdebug_get_tracefile_name());
var_dump(xdebug_stop_trace());

// pid
var_dump(xdebug_start_trace("/tmp/trace-%p"));
var_dump(xdebug_stop_trace());

// random
var_dump(xdebug_start_trace("/tmp/trace-%r"));
var_dump(xdebug_stop_trace());

// script name
var_dump(xdebug_start_trace("/tmp/trace-%s"));
var_dump(xdebug_stop_trace());

// timestamp (sec)
var_dump(xdebug_start_trace("/tmp/trace-%t"));
var_dump(xdebug_stop_trace());

// timestamp (microseconds)
var_dump(xdebug_start_trace("/tmp/trace-%u"));
var_dump(xdebug_stop_trace());

// $_SERVER['HTTP_HOST']
var_dump(xdebug_start_trace("/tmp/trace-%H"));
var_dump(xdebug_stop_trace());

// $_SERVER['REQUEST_URI']
var_dump(xdebug_start_trace("/tmp/trace-%R"));
var_dump(xdebug_stop_trace());

// $_SERVER['UNIQUE_ID']
var_dump(xdebug_start_trace("/tmp/trace-%U"));
var_dump(xdebug_stop_trace());

// session id
var_dump(xdebug_start_trace("/tmp/trace-%S"));
var_dump(xdebug_stop_trace());

// Literal %
var_dump(xdebug_start_trace("/tmp/trace-%%"));
var_dump(xdebug_stop_trace());
