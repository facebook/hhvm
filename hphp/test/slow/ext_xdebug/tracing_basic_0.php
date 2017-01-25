<?
require('tracing_basic.inc');

$files = [];
try {
  // Default
  var_dump(xdebug_start_trace());
  var_dump(xdebug_get_tracefile_name());
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // Naked
  var_dump(xdebug_start_trace(null, XDEBUG_TRACE_NAKED_FILENAME));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // Non default
  var_dump(xdebug_start_trace("/tmp/trace"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // crc32
  var_dump(xdebug_start_trace("/tmp/trace-%c"));
  var_dump(xdebug_get_tracefile_name());
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // pid
  var_dump(xdebug_start_trace("/tmp/trace-%p"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // random
  var_dump(xdebug_start_trace("/tmp/trace-%r"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // script name
  var_dump(xdebug_start_trace("/tmp/trace-%s"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // timestamp (sec)
  var_dump(xdebug_start_trace("/tmp/trace-%t"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // timestamp (microseconds)
  var_dump(xdebug_start_trace("/tmp/trace-%u"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // $_SERVER['HTTP_HOST']
  var_dump(xdebug_start_trace("/tmp/trace-%H"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // $_SERVER['REQUEST_URI']
  var_dump(xdebug_start_trace("/tmp/trace-%R"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // $_SERVER['UNIQUE_ID']
  var_dump(xdebug_start_trace("/tmp/trace-%U"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // session id
  var_dump(xdebug_start_trace("/tmp/trace-%S"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;

  // Literal %
  var_dump(xdebug_start_trace("/tmp/trace-%%"));
  $f = xdebug_stop_trace();
  var_dump($f);
  $files[] = $f;
} finally {
  // Cleanup
  @array_map("unlink", $files);
}
