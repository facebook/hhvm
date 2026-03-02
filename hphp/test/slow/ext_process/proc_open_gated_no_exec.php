<?hh

/**
 * Test that proc_open rejects calls when Server.AllowExec is false,
 * even if the caller is in the approved callers list.
 */
<<__EntryPoint>>
function test_allow_exec_disabled(): void {
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
