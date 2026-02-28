<?hh

function VS($x, $y) :mixed{
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) :mixed{ VS($x !== false, true); }



//////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main_ext_process() :mixed{
$pid = pcntl_fork();
if ($pid == 0) {
  exit(123);
}
$status = null;
pcntl_wait(inout $status);

$pri = pcntl_getpriority();
VERIFY($pri);
if ($pri < 15) {
  VS(pcntl_setpriority(15), true);
  VS(pcntl_getpriority(), 15);
} else {
  var_dump(true, true);
}

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_wait(inout $status);
VS($status, 0x1200);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, inout $status);
VS($status, 0x1200);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x80);
}
pcntl_waitpid(0, inout $status);
VS(pcntl_wexitstatus($status), 0x80);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, inout $status);
VERIFY(pcntl_wifexited($status));

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, inout $status);
VERIFY(!pcntl_wifsignaled($status));

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, inout $status);
VERIFY(!pcntl_wifstopped($status));

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, inout $status);
VS(pcntl_wstopsig($status), 0x12);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, inout $status);
VS(pcntl_wtermsig($status), 0);
}
