<?php

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }

//////////////////////////////////////////////////////////////////////

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

$errout = tempnam('/tmp', 'vmtesterrout');
unlink($errout);

$descriptorspec =
  array(array("pipe", "r"),
        array("pipe", "w"),
        array("file", $errout, "a"));

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
        array("file", $errout, "a"));
$cwd = "/tmp";

$process = proc_open("sh", $descriptorspec, $pipes, $cwd);
VERIFY($process != false);

fprintf($pipes[0], "echo Sup");
fclose($pipes[0]);
fpassthru($pipes[1]);
VS(proc_close($process), 0);

$descriptorspec =
  array(array("pipe", "r"),
        array("pipe", "w"),
        array("file", $errout, "a"));
$process = proc_open('cat', $descriptorspec, $pipes);
VERIFY($process != false);
VERIFY(proc_terminate($process));
// still need to close it, not to leave a zombie behind
proc_close($process);

$process = proc_open('cat', $descriptorspec, $pipes);
VERIFY($process != false);
$ret = proc_get_status($process);
VS($ret['command'], 'cat');
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
