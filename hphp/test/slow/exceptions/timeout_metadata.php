<?hh

set_time_limit(1);
set_error_handler(($errno, $errstr, $file, $line, $context, $trace) ==> {
  foreach ($trace as $frame) {
    $metadata = $frame['metadata'] ?? null;
    if ($metadata) var_dump($metadata);
  }
  return false;
});

function foo() {
  HH\set_frame_metadata('hello world');
  while (true) {
    sleep(1);
  }
}

foo();
