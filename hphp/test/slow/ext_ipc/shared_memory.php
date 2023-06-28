<?hh


<<__EntryPoint>>
function main_shared_memory() :mixed{
$ret = shm_attach(0xDEADBEEF);
if ($ret === false) { echo "failed\n"; exit(1); }
shm_remove($ret); // just in case its left over from an earlier run
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

$status = null;
pcntl_waitpid($pid, inout $status);
var_dump($status);

// Verify that shm_remove_var worked
$ret = shm_get_var($index, 1234);
var_dump($ret === false);

shm_remove($index);
}
