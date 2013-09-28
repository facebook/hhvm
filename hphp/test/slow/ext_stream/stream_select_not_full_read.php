<?php

$descriptorspec = array(
  array("pipe", "r"),
  array("pipe", "w"),
  array("pipe", "a"),
);
$process = proc_open('cat', $descriptorspec, $io);
fwrite($io[0], "a\nb\nc\n");

// Just to have another thing in the $r array that has no data
$process2 = proc_open('cat', $descriptorspec, $io2);

while (!feof($io[1])) {
  $r = array($io[1], $io2[1]);
  $w = $e = null;
  $i = stream_select($r, $w, $e, 1);
  var_dump($i);
  if ($i) {
    foreach ($r as $resource) {
      var_dump(fgets($resource));
    }
  } else {
    break;
  }
}
