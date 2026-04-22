<?hh

/**
 * Test that proc_open, popen, and pclose reject calls when Server.AllowExec
 * is false, even if the caller is in the approved callers list.
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
    echo "FAIL: proc_open did not throw\n";
  } catch (Exception $e) {
    echo "proc_open: ".get_class($e).': '.$e->getMessage()."\n";
  }

  try {
    popen('echo should_not_run', 'r');
    echo "FAIL: popen did not throw\n";
  } catch (Exception $e) {
    echo "popen: ".get_class($e).': '.$e->getMessage()."\n";
  }

  try {
    pclose(null);
    echo "FAIL: pclose did not throw\n";
  } catch (Exception $e) {
    echo "pclose: ".get_class($e).': '.$e->getMessage()."\n";
  }
}
