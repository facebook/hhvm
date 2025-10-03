<?hh

function test_core(WatchmanInstance $wminst): void {
  if ((int)HH\ext_watchman_version()) {
    print "PASSED version get test\n";
  } else {
    print "FAILED version get test\n";
  }

  $sock = $wminst->getFullSockName();
  // Test one-shot
  print("Testing one-shot\n");
  $version = HH\Asio\join(HH\watchman_run(
    '["version", { "optional": ["relative_root"] }]',
    $sock,
  ));
  if (!HH\Lib\Str\contains($version, '"capabilities":{"relative_root":true}')) {
    throw new Exception("FAIL ('.var_export($version).')\n");
  } else {
    print("PASS\n");
  }

  // Make sure we fail gracefully if we connect to an invalid socket
  print("Checking connecting to a bad socket is graceful\n");
  $saw_exception = false;
  try {
    HH\watchman_run(
    '["version", { "optional": ["relative_root"] }]',
      "garbage",
    )
      |> HH\Asio\join($$);
  } catch(Exception $e) {
    print('Got exception: '.$e->getMessage()."\n");
    $saw_exception = true;
  }
  if ($saw_exception) {
    print("PASS\n");
  } else {
    print("FAIL\n");
  }
}
<<__EntryPoint>>
function entrypoint_ext_watchman(): void {
  require_once 'constants.inc';
  require_once 'callback.inc';
  require_once 'wminst.inc';

  $tmpdir = tempnam(sys_get_temp_dir(), 'wmt');
  unlink($tmpdir);
  if (!mkdir($tmpdir)) {
    throw new Exception("FAIL failed creating dir '$tmpdir'\n");
  }
  $wminst = null;
  try {
    if (!chdir($tmpdir)) {
      throw new Exception("FAIL (creating temporary directory)\n");
    }
    $wminst = new WatchmanInstance($tmpdir);
    test_core($wminst);
  } finally {
    apc_delete('stress_counter');  // Stops async callback_checksub() if running
    $wminst->terminateProcess();
    if (is_dir($tmpdir)) {
      foreach (glob($tmpdir.'/*') as $file) {
        unlink($file);
      }
      rmdir($tmpdir);
    }
  }
}
