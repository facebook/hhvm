<?hh

function test_me($desc) :mixed{
  $pipes = null;
  $child_status = null;
  $process = proc_open(__DIR__."/test_proc_open.sh", darray($desc), inout $pipes);
  $status = proc_get_status($process);
  pcntl_waitpid($status["pid"], inout $child_status);
}


<<__EntryPoint>>
function main_proc_open() :mixed{
$desc = vec[vec["file", "php://stdin", "r"]];
test_me($desc);

$desc = vec[vec["file", __DIR__ . "/test_proc_open.txt", "r"]];
test_me($desc);

$desc = vec[vec["file", __DIR__, "r"]];
test_me($desc);

$desc = vec[vec["file", "php://fd/0", "r"]];
test_me($desc);

$desc = vec[vec["file", "php://temp", "r"]];
test_me($desc);

$desc = vec[vec["file", "php://memory", "r"]];
test_me($desc);
}
