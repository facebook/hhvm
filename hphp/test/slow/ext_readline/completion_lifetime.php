<?hh

function check_completion(bool $throws): void {
  $command = escapeshellarg(HH\__internal\hhvm_binary()).
    ' --no-config -vEval.Jit=false --count='.($throws ? '1' : '2').' '.
    escapeshellarg(__DIR__.'/completion_lifetime.inc').
    ($throws ? ' throw' : '').' 2>&1';
  $pipes = dict[];
  $process = proc_open(
    $command,
    dict[
      0 => tuple('pipe', 'r'),
      1 => tuple('pipe', 'w'),
    ],
    inout $pipes,
  );
  fwrite($pipes[0], "he\t\nhe\t\n");
  fclose($pipes[0]);
  $output = stream_get_contents($pipes[1]);
  fclose($pipes[1]);
  var_dump(proc_close($process));
  foreach (explode("\n", $output) as $line) {
    if (strpos($line, 'completed: ') === 0 ||
        strpos($line, 'caught: ') === 0 ||
        strpos($line, 'vec(1) refcount(') === 0) {
      echo $line, "\n";
    }
  }
}

<<__EntryPoint>>
function main(): void {
  check_completion(false);
  check_completion(true);
}
