<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

$pid = pcntl_fork();
if ($pid == 0) {
  exit(123);
}
pcntl_wait($status);

VS(pcntl_getpriority(), 0);
VERIFY(pcntl_setpriority(0));

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_wait($status);
VS($status, 0x1200);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VS($status, 0x1200);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x80);
}
pcntl_waitpid(0, $status);
VS(pcntl_wexitstatus($status), 0x80);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VERIFY(pcntl_wifexited($status));

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VERIFY(!pcntl_wifsignaled($status));

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VERIFY(!pcntl_wifstopped($status));

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VS(pcntl_wstopsig($status), 0x12);

$pid = pcntl_fork();
if ($pid == 0) {
  exit(0x12);
}
pcntl_waitpid(0, $status);
VS(pcntl_wtermsig($status), 0);

$output = shell_exec("echo hello");
VS($output, "hello\n");

chdir("/tmp/");
VS(shell_exec("/bin/pwd"), "/tmp\n");

pcntl_signal_dispatch();

$last_line = exec("echo hello; echo world;", $output, $ret);
VS($output, array("hello", "world"));
VS($last_line, "world");
VS($ret, 0);

chdir("/tmp/");
VS(exec("/bin/pwd"), "/tmp");

echo "heh\n";

passthru("echo hello; echo world;", $ret);
VS($ret, 0);

chdir("/tmp/");
passthru("/bin/pwd");

$last_line = system("echo hello; echo world;", $ret);
VS($last_line, "world");
VS($ret, 0);

chdir("/tmp/");
VS(system("/bin/pwd"), "/tmp");

$descriptorspec =
  array(array("pipe", "r"),
        array("pipe", "w"),
        array("file", "/tmp/error-output.txt", "a"));

putenv("inherit_me=please");
$process = proc_open('echo $inherit_me', $descriptorspec, $pipes);
VERIFY($process != false);

fpassthru($pipes[1]);

VS(proc_close($process), 0);

// Ensure that PATH makes it through too
$process = proc_open('echo $PATH', $descriptorspec, $pipes);
VERIFY($process != false);

VERIFY(strlen(fgets($pipes[1])) > 2);

VS(proc_close($process), 0);

$descriptorspec =
  array(array("pipe", "r"),
        array("pipe", "w"),
        array("file", "/tmp/error-output.txt", "a"));
$cwd = "/tmp";
$env = array("some_option" => "aeiou");

$process = proc_open("php", $descriptorspec, $pipes, $cwd, $env);
VERIFY($process != false);

fprintf($pipes[0], "<?php print(getenv('some_option')); ?>");
fclose($pipes[0]);
fpassthru($pipes[1]);
VS(proc_close($process), 0);

$descriptorspec =
  array(array("pipe", "r"),
        array("pipe", "w"),
        array("file", "/tmp/error-output.txt", "a"));
$process = proc_open('php', $descriptorspec, $pipes);
VERIFY($process != false);
VERIFY(proc_terminate($process));
// still need to close it, not to leave a zombie behind
proc_close($process);

$process = proc_open('php', $descriptorspec, $pipes);
VERIFY($process != false);
$ret = proc_get_status($process);
VS($ret['command'], 'php');
VERIFY($ret['pid'] > 0);
VERIFY($ret['running']);
VERIFY(!$ret['signaled']);
VS($ret['exitcode'], -1);
VS($ret['termsig'], 0);
VS($ret['stopsig'], 0);
proc_close($process);

VERIFY(proc_nice(0));

VS(escapeshellarg("\""), "'\"'");

VS(escapeshellcmd("perl \""), "perl \\\"");
