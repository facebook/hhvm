<?hh

<<__EntryPoint>> function main(): void {
require(__DIR__ . '/common.inc');
/*
 * Not enabled test: verify that if --mode vsdebug is not specified, the
 * VSDebug extension is not listening and the script executes without waiting
 * for the debugger.
 */
$descriptorspec = dict[
   0 => vec["pipe", "r"], // stdin
   1 => vec["pipe", "w"], // stdout
   2 => vec["pipe", "w"]  // stderr
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
