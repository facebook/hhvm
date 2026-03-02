<?hh

/**
 * Test that shell_exec, exec, passthru, and system are rejected even when the
 * caller is listed in the Eval.ProcOpenGatedApprovedCallers allowlist.
 * Only proc_open should be permitted for approved callers.
 */
<<__EntryPoint>>
function test_ungated_apis_rejected(): void {
  // shell_exec should be rejected
  try {
    shell_exec('echo should_not_run');
    echo "FAIL: shell_exec did not throw\n";
  } catch (Exception $e) {
    echo "shell_exec: ".get_class($e).': '.$e->getMessage()."\n";
  }

  // exec should be rejected
  try {
    $output = null;
    $ret = null;
    exec('echo should_not_run', inout $output, inout $ret);
    echo "FAIL: exec did not throw\n";
  } catch (Exception $e) {
    echo "exec: ".get_class($e).': '.$e->getMessage()."\n";
  }

  // passthru should be rejected
  try {
    $ret = null;
    passthru('echo should_not_run', inout $ret);
    echo "FAIL: passthru did not throw\n";
  } catch (Exception $e) {
    echo "passthru: ".get_class($e).': '.$e->getMessage()."\n";
  }

  // system should be rejected
  try {
    $ret = null;
    system('echo should_not_run', inout $ret);
    echo "FAIL: system did not throw\n";
  } catch (Exception $e) {
    echo "system: ".get_class($e).': '.$e->getMessage()."\n";
  }

  // proc_open should succeed for this approved caller
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
  echo "proc_open: ".trim($output).", exit=$exit\n";
}
