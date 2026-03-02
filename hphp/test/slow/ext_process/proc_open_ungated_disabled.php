<?hh

/**
 * Test that proc_open rejects calls when Eval.AllowUngatedExec is false.
 */
<<__EntryPoint>>
function test_ungated_exec_disabled(): void {
  $descriptorspec = vec[
    vec["pipe", "r"],
    vec["pipe", "w"],
    vec["pipe", "w"],
  ];
  $pipes = null;
  try {
    proc_open(
      'echo should_not_run',
      darray($descriptorspec),
      inout $pipes,
    );
    echo "FAIL: expected exception was not thrown\n";
  } catch (Exception $e) {
    echo get_class($e).': '.$e->getMessage()."\n";
  }
}
