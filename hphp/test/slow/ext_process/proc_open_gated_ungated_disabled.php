<?hh

/**
 * Test that proc_open succeeds even when Eval.AllowUngatedExec is
 * false, as long as the caller is in the approved callers list.
 */
<<__EntryPoint>>
function test_gated_with_ungated_disabled(): void {
  $descriptorspec = vec[
    vec["pipe", "r"],
    vec["pipe", "w"],
    vec["pipe", "w"],
  ];
  $pipes = null;
  $process = proc_open(
    'echo gated_still_works',
    darray($descriptorspec),
    inout $pipes,
  );
  if ($process === false) {
    echo "FAIL: proc_open returned false\n";
    return;
  }
  $output = stream_get_contents($pipes[1]);
  fclose($pipes[1]);
  fclose($pipes[2]);
  fclose($pipes[0]);
  $exit = proc_close($process);
  echo trim($output)."\n";
  echo "exit=$exit\n";
}
