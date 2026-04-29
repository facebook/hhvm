<?hh

/**
 * Test that the gating logic skips the FlibSL\PHP\proc_open wrapper
 * (configured via Eval.ProcOpenGatedSkipCallers) and attributes the call
 * to the real caller behind it.
 */
<<__EntryPoint>>
function test_gated_skip_wrapper(): void {
  require_once __DIR__.'/proc_open_flibsl_wrapper.inc';

  $descriptorspec = vec[
    vec["pipe", "r"],
    vec["pipe", "w"],
    vec["pipe", "w"],
  ];
  $pipes = null;
  $process = \FlibSL\PHP\proc_open(
    'echo wrapper_skipped_ok',
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
