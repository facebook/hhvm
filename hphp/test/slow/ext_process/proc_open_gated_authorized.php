<?hh

/**
 * Test that proc_open succeeds when AllowUngatedExec is false and the caller
 * is listed in the Eval.ProcOpenGatedApprovedCallers allowlist (free function).
 */
<<__EntryPoint>>
function test_authorized(): void {
  $descriptorspec = vec[
    vec["pipe", "r"],
    vec["pipe", "w"],
    vec["pipe", "w"],
  ];
  $pipes = null;
  $process = proc_open(
    'echo gated_ok',
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
