<?hh

function VS($x, $y) {
  var_dump($x === $y);
  if ($x !== $y) { echo "Failed: $y\n"; echo "Got: $x\n";
                   var_dump(debug_backtrace()); }
}
function VERIFY($x) { VS($x != false, true); }


//////////////////////////////////////////////////////////////////////

<<__EntryPoint>>
function main_lwp() {
$output = shell_exec("echo hello");
VS($output, "hello\n");

chdir("/tmp/");
VS(shell_exec("/bin/pwd"), "/tmp\n");

pcntl_signal_dispatch();

$ret = -1;
$last_line = exec("echo hello; echo world;", inout $output, inout $ret);
VS($output, varray["hello", "world"]);
VS($last_line, "world");
VS($ret, 0);

chdir("/tmp/");
VS(exec("/bin/pwd", inout $output, inout $ret), "/tmp");

echo "heh\n";

passthru("echo hello; echo world;", inout $ret);
VS($ret, 0);

chdir("/tmp/");
passthru("/bin/pwd", inout $ret);

$last_line = system("echo hello; echo world;", inout $ret);
VS($last_line, "world");
VS($ret, 0);

chdir("/tmp/");
VS(system("/bin/pwd", inout $ret), "/tmp");

$errout = tempnam('/tmp', 'vmtesterrout');
unlink($errout);

$descriptorspec =
  varray[varray["pipe", "r"],
        varray["pipe", "w"],
        varray["file", $errout, "a"]];

putenv("inherit_me=please");
$pipes = null;
$process = proc_open('echo $inherit_me', $descriptorspec, inout $pipes);
VERIFY($process != false);

fpassthru($pipes[1]);

VS(proc_close($process), 0);

// Ensure that PATH makes it through too
$process = proc_open('echo $PATH', $descriptorspec, inout $pipes);
VERIFY($process != false);

VERIFY(strlen(fgets($pipes[1])) > 2);

VS(proc_close($process), 0);

$descriptorspec =
  varray[varray["pipe", "r"],
        varray["pipe", "w"],
        varray["file", $errout, "a"]];
$cwd = "/tmp";

$process = proc_open("sh", $descriptorspec, inout $pipes, $cwd);
VERIFY($process != false);

fprintf($pipes[0], "echo Sup");
fclose($pipes[0]);
fpassthru($pipes[1]);
VS(proc_close($process), 0);

$descriptorspec =
  varray[varray["pipe", "r"],
        varray["pipe", "w"],
        varray["file", $errout, "a"]];
$process = proc_open('cat', $descriptorspec, inout $pipes);
VERIFY($process != false);
VERIFY(proc_terminate($process));
// still need to close it, not to leave a zombie behind
proc_close($process);

$process = proc_open('cat', $descriptorspec, inout $pipes);
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

$nullbyte = "echo abc\n\0command";
$return_var = -1;
VS(passthru($nullbyte, inout $return_var), null);
VS(system($nullbyte, inout $return_var), "");
$nullbyteout = null;
VS(exec($nullbyte, inout $nullbyteout, inout $return_var), "");
VS($nullbyteout, varray[]);
VS(shell_exec($nullbyte), null);
$process = proc_open($nullbyte, varray[], inout $pipes);
VS($process, false);
}
