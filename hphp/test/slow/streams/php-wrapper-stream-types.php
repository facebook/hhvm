<?php

function main() {
  $to_open = array(
    array('php://stdin', 'r'),
    array('php://stdout', 'w'),
    array('php://stderr', 'w'),
    array('php://fd/1', 'w'), // stdout
    array('php://temp', 'rw+'),
    array('php://memory', 'rw+'),
    array('php://input', 'r'),
    array('php://output', 'w'),
  );
  foreach ($to_open as $target) {
    $stream = call_user_func_array('fopen', $target);
    $metadata = stream_get_meta_data($stream);
    var_dump(
      array(
        'target' => $target[0],
        'wrapper' => $metadata['wrapper_type'],
        'stream' => $metadata['stream_type'],
      )
    );
  }
}

main();
