<?hh

function test_me($desc) {
  $pipes = null;
  $child_status = null;
  $process = proc_open(__DIR__."/test_proc_open.sh", $desc, inout $pipes);
  $status = proc_get_status($process);
  pcntl_waitpid($status["pid"], inout $child_status);
}


<<__EntryPoint>>
function main_proc_open() {
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
}
