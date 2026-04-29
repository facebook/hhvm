<?hh

/**
 * Test that the gating logic skips the FlibSL\PHP\proc_open wrapper
 * (configured via Eval.ProcOpenGatedSkipCallers) and reports the real
 * caller in the error message.
 */
<<__EntryPoint>>
function test_gated_skip_wrapper_unauthorized(): void {
  require_once __DIR__.'/proc_open_flibsl_wrapper.inc';

  $descriptorspec = vec[
    vec["pipe", "r"],
    vec["pipe", "w"],
    vec["pipe", "w"],
  ];
  $pipes = null;
  try {
    \FlibSL\PHP\proc_open(
      'echo should_not_run',
      darray($descriptorspec),
      inout $pipes,
    );
    echo "FAIL: expected exception was not thrown\n";
  } catch (\Exception $e) {
    echo get_class($e).': '.$e->getMessage()."\n";
  }
}
