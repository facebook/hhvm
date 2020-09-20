<?hh

<<__EntryPoint>>
function main_proc_terminate() {
$cmd = 'sleep 3';
$descriptors = darray[];

// Don't add signals that would cause the process to abort, the output will
// depend on whether it dumps its core.
$signals = varray[
  2.2,    // SIGINT
  '9',    // SIGKILL
  'herp', // invalid, does nothing
  -4,     // invalid
];

foreach ($signals as $signal) {
  $pipes = null;
  $process = proc_open($cmd, $descriptors, inout $pipes);
  $result = false;
  try { $result = proc_terminate($process, $signal); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
  var_dump($result);
}
}
