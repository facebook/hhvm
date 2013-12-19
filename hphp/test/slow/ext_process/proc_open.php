<?php

function test_me($desc) {
  $pipes = null;
  $process = proc_open(__DIR__."/test_proc_open.sh", $desc, $pipes);
  $status = proc_get_status($process);
  pcntl_waitpid($status["pid"], $child_status);
}

$desc = array(array("file", "php://stdin", "r"));
test_me($desc);

$desc = array(array("file", __DIR__ . "/test_proc_open.txt", "r"));
test_me($desc);

$desc = array(array("file", __DIR__, "r"));
test_me($desc);

$desc = array(array("file", "php://fd/0", "r"));
test_me($desc);

$desc = array(array("file", "php://temp", "r"));
test_me($desc);

$desc = array(array("file", "php://memory", "r"));
test_me($desc);
