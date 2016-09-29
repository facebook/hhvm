<?php

$ret = shm_attach(0xDEADBEEF);
if ($ret === false) { echo "failed\n"; exit(1); }
$index = $ret;
var_dump(shm_has_var($index, 1234));
shm_put_var($index, 1234, "test");
var_dump(shm_has_var($index, 1234));

$pid = pcntl_fork();
if ($pid == 0) {
  $ret = shm_attach($index);
  $ret = shm_get_var($index, 1234);
  if ($ret !== "test") {
    echo "oops\n";
    exit(1);
  }
  shm_remove_var($index, 1234);
  shm_detach($index);
  exit(0);
}

// Verifying shm_remove_var worked, this is not sure test though.
$ret = shm_get_var($index, 1234);
for ($i = 0; $i < 1000; $i++) {
  if ($ret === false) break;
  usleep(1000);
  $ret = shm_get_var($index, 1234);
}
var_dump($ret === false);

shm_remove($index);

pcntl_waitpid($pid, $status);

