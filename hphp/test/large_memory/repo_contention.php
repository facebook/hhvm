<?php

function foobar($i) {
  usleep(500*1000);
}

function main($num) {
  $cmd = file_get_contents("/proc/self/cmdline");
  $cmd = str_replace("\000", " ", $cmd);
  $start = microtime(true);

  echo "Starting $num procs\n";
  for ($i = 0; $i < $num; $i++) {
    $handles[] = proc_open($cmd." $i", array(), $pipes);
  }
  while ($handles) {
    foreach ($handles as $i => $handle) {
      $status = proc_get_status($handle);
      if (!$status['running']) {
        proc_close($handle);
        unset($handles[$i]);
      }
    }
  }
  echo "Done $num procs\n";
  return microtime(true) - $start;
}

if (count($argv) > 1) {
  foobar($argv[1]);
} else {
  $x10 = main(10);
  $x200 = main(200);
  echo $x10 * 20 < $x200 ? "Failed since $x10 * 20 < $x200" : "Passed", "\n";
}
