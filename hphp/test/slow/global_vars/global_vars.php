<?hh
<<__EntryPoint>>
function entrypoint_global_vars(): void {
  if (\HH\global_get('argc') > 1) {
    echo "---- " . ini_get('variables_order') . " ----\n";

    var_dump(count($_SERVER) > 0);
    var_dump(count($_ENV) > 0);
    exit;
  }

  $args = varray['', 'G', 'P', 'E', 'S', 'ES', 'ESCGP', 'CG'];

  $processes = vec[];
  foreach($args as $arg) {
    $descriptorspec = darray[
       0 => varray['pipe', 'r'],
       1 => varray['pipe', 'w'],
       2 => varray['pipe', 'w']];

    // -dvariables_order=  didn't consist with PHP
    // it should be fixed in ini level. so compatible with it for now
    $arg_str = $arg ? " -dvariables_order=$arg " : " ";

    $cmd = PHP_BINARY . $arg_str . __FILE__ . " dummy";

    $pipes = null;
    $proc = proc_open($cmd, $descriptorspec, inout $pipes);
    if (!$proc) {
      echo "Failed to open: $cmd\n";
      exit -1;
    }
    $processes[] = tuple($proc, $pipes);
  }

  foreach ($processes as $proc) {
    echo stream_get_contents($proc[1][1]);
    fclose($proc[1][1]);
    proc_close($proc[0]);
  }
}
