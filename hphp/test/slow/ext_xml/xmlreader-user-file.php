<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; }
}


class StreamWrapper {
  private $file;
  function stream_open($path, $mode, $options, &$opened_path) {
    return true;
  }
  function stream_read($count) { return ''; }
}

stream_wrapper_register('streamwrapper', 'StreamWrapper');


ini_set( 'display_errors', 'stderr' );

$reader = new XMLReader();
var_dump($reader->open('streamwrapper://'));

var_dump($reader->close());
