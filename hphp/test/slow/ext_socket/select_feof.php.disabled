<?php

$descriptors = array(
  array('pipe', 'r'), // stdin
  array('pipe', 'w'), // stdout
  array('pipe', 'w'), // stderr
);

$process = proc_open('echo "hi"', $descriptors, $pipes);

// Hopefully echo is done in 1 second...
sleep(1);

for ($i = 0; $i < 2; $i++) {
  $r = array($pipes[1], $pipes[2]);
  $w = null;
  $e = null;
  var_dump(stream_select($r, $w, $e, 0, 200000));

  foreach ($r as $pipe) {
    var_dump(
      array_search($pipe, $pipes),
      fread($pipe, 8192),
      feof($pipe)
    );
  }
}
