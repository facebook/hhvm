<?hh

require(__DIR__ . '/common.inc');
<<__EntryPoint>> function main(): void {
/*
 * Not enabled test: verify that if --mode vsdebug is not specified, the
 * VSDebug extension is not listening and the script executes without waiting
 * for the debugger.
 */
$descriptorspec = darray[
   0 => varray["pipe", "r"], // stdin
   1 => varray["pipe", "w"], // stdout
   2 => varray["pipe", "w"]  // stderr
];

$cmd = getHhvmPath() . " " . __DIR__ . "/not_enabled.php.test";
$pipes = null;
$process = proc_open($cmd, $descriptorspec, inout $pipes);
if (!is_resource($process)) {
  throw new UnexpectedValueException("Failed to open child process!");
}

$stdout = $pipes[1];
$stderr = $pipes[2];
echo stream_get_contents($stdout);
echo stream_get_contents($stderr);

foreach ($pipes as $pipe) {
  fclose($pipe);
}

proc_close($process);
}
