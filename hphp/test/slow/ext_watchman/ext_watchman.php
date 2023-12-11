<?hh

function waitFor(string $filename, WatchmanInstance $wminst): void {
  print("Waiting for $filename\n");
  $timeout = 5;
  while($timeout && !file_exists($filename)) {
    sleep(1);
    $timeout--;
  }
  if ($timeout) {
    print("PASS\n");
  } else {
    $wminst->dumpLog();
    throw new Exception("FAIL (timeout on $filename)\n");
  }
}

function test_core(WatchmanInstance $wminst): void {
  if ((int)HH\ext_watchman_version()) {
    print "PASSED version get test\n";
  } else {
    print "FAILED version get test\n";
  }

  $sock = $wminst->getFullSockName();
  // Test one-shot
  print("Testing one-shot\n");
  $version = HH\Asio\join(HH\watchman_query(
    vec['version', dict['optional' => vec['relative_root']]],
    $sock,
  ));
  if (
    !isset('capabilities', $version) ||
    $version['capabilities'] !== dict['relative_root' => true]
  ) {
    throw new Exception("FAIL ('.var_export($version).')\n");
  } else {
    print("PASS\n");
  }

  print("Testing subscription\n");
  file_put_contents(TEST_INITIAL_FILE, "");
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'callback_files',
    $sock,
  ) |> HH\Asio\join($$);
  print("Subscribed\n");
  waitFor(TEST_FILE_GOT_FRESH, $wminst);
  waitFor(TEST_FILE_GOT_UPDATE, $wminst);
  print("Unsubscribing\n");
  $str = HH\watchman_unsubscribe(SUB_NAME) |> HH\Asio\join($$);
  print("PASS\n");
  print("Confirming subscription no longer exists\n");
  if (HH\watchman_check_sub(SUB_NAME)) {
    throw new Exception("FAIL (still reporting subscribed)\n");
  }
  print("PASS\n");

  print("Test corrupt subscription callback\n");
  $tmpcode = tempnam(sys_get_temp_dir(), 'cd1');
  chmod($tmpcode, 0666);
  file_put_contents($tmpcode, '<?hh function tmpCB($_, $_, $_, $_, $_){}');
  require $tmpcode;
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'tmpCB',
    $sock,
  ) |> HH\Asio\join($$);
  file_put_contents($tmpcode, '<?hh function tmpCB($_, $_, $_, $_, $_){asdf}');
  file_put_contents($wminst->getRepoRoot().'/blah.php', 'X');
  sleep(2);
  $breakpass = false;
  try {
    HH\watchman_check_sub(SUB_NAME);
  } catch (Exception $e) {
    print('Got expected exception: '.$e->getMessage()."\n");
    $breakpass = true;
  }
  if ($breakpass) {
    print("PASS\n");
    unset($breakpass);
  } else {
    print("FAIL\n");
  }
  HH\watchman_unsubscribe(SUB_NAME) |> HH\Asio\join($$);

  print("Test delete subscription callback\n");
  $tmpcode = tempnam(sys_get_temp_dir(), 'cd2');
  chmod($tmpcode, 0666);
  file_put_contents($tmpcode, '<?hh function tmpCB1($_, $_, $_, $_, $_){}');
  require $tmpcode;
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'tmpCB1',
    $sock,
  ) |> HH\Asio\join($$);
  unlink($tmpcode);
  file_put_contents($wminst->getRepoRoot().'/blah.php', 'X');
  sleep(2);
  try {
    HH\watchman_check_sub(SUB_NAME);
  } catch (Exception $e) {
    print('Got expected exception: '.$e->getMessage()."\n");
    $breakpass = true;
  }
  if ($breakpass) {
    print("PASS\n");
    unset($breakpass);
  } else {
    print("FAIL\n");
  }
  HH\watchman_unsubscribe(SUB_NAME) |> HH\Asio\join($$);

  print("Stress test\n");
  apc_store('stress_counter', 0);
  fb_call_user_func_array_async(
    __DIR__.'/callback.inc',
    'callback_checksub',
    vec[SUB_NAME],
  );
  // Success for this test is just not crashing HHVM
  $subscribed = false;
  $subscribe_c = 0;
  $unsubscribe_c = 0;
  $touch_c = 0;
  $last_subscribe = null;
  $last_unsubscribe = null;
  $exception_count = vec[0, 0, 0];
  $op_count = vec[0, 0, 0];
  srand(1);
  for ($i = 0; $i < 30000; $i++) {
    if ($i % 1000 === 0) {
      print(".");
      invariant(
        ($op_count[0] === 0 && $op_count[1] === 0)
        || (
          $exception_count[0] !== $op_count[0]
          && $exception_count[1] !== $op_count[1]
        ),
        "All recent subscribe/unsubscribe operations failed",
      );
      $exception_count = vec[0, 0, 0];
      $op_count = vec[0, 0, 0];
    }
    $op = rand() % 3;
    $op_count[$op]++;
    try {
      switch($op)
      {
        case 0: // subscribe
          $subscribe_c++;
          $last_subscribe = HH\watchman_subscribe(
            '{"fields": ["name"], "expression": ["exists"]}',
            $wminst->getRepoRoot(),
            SUB_NAME,
            'callback_stress',
            $sock,
          );
          break;

        case 1: // unsubscribe
          $unsubscribe_c++;
          $last_unsubscribe = HH\watchman_unsubscribe(SUB_NAME);
          break;

        case 2: // touch a file
          $touch_c++;
          file_put_contents($wminst->getRepoRoot() . '/afile', 'X');
          break;
      }
    } catch (Exception $e) {
      $exception_count[$op]++;
    }
    usleep(rand() % 1000);
  }
  try {
    if ($last_subscribe) {
      HH\Asio\join($last_subscribe);
    }
    if ($last_unsubscribe) {
      HH\Asio\join($last_unsubscribe);
    }
  } catch(Exception $_) { }
  if (HH\watchman_check_sub(SUB_NAME)) {
    HH\Asio\join(HH\watchman_unsubscribe(SUB_NAME));
  }
  $hit_c = __hhvm_intrinsics\apc_fetch_no_check('stress_counter');
  if (!$subscribe_c || !$unsubscribe_c || !$touch_c || !$hit_c) {
    print("\nFAIL ($subscribe_c, $unsubscribe_c, $touch_c, $hit_c)\n");
  } else {
    print("\nPASS ($subscribe_c, $unsubscribe_c, $touch_c, $hit_c)\n");
  }
  apc_delete('stress_counter');  // Stops async callback_checksub()
  sleep(1);

  // Sync testing is based on the fact there will immediately be an event to
  // process as Watchman will send us an "initial" update straight away. There
  // should then be no further updates.
  print("Testing syncing\n");
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'callback_sync',
    $sock,
  ) |> HH\Asio\join($$);
  print("Subscribed, sleeping for 1 second...\n");
  sleep(1);
  print("Sync expecting timeout after 100ms...\n");
  $synced = HH\watchman_sync_sub(SUB_NAME, 100) |> HH\Asio\join($$);
  if ($synced) {
    print("FAIL\n");
  } else {
    print("PASS\n");
  }
  print("Sync expecting no timeout...\n");
  if (HH\Asio\join(HH\watchman_sync_sub(SUB_NAME, 10000))) {
    print("PASS\n");
  } else {
    print("FAIL\n");
  }
  print("Unsubscribing\n");
  $str = HH\watchman_unsubscribe(SUB_NAME) |> HH\Asio\join($$);

  print("Check catching exception raised in a subscription callback\n");
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'callback_exception',
    $sock,
  )|> HH\Asio\join($$);
  sleep(2);
  $saw_exception = false;
  try {
    HH\watchman_check_sub(SUB_NAME);
  } catch(Exception $e) {
    print('Got exception: '.$e->getMessage()."\n");
    $saw_exception = true;
  }
  if ($saw_exception) {
    print("PASS\n");
  } else {
    print("FAIL\n");
  }
  HH\watchman_unsubscribe(SUB_NAME) |> HH\Asio\join($$);

  // Check we can unsubscribe while a callback is in progress. The easiest way
  // synthesize this event is to unsubscribe in a callback itself.
  print("Check unsubscribing while callback in progress\n");
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'callback_unsubscribe',
    $sock,
  )|> HH\Asio\join($$);
  HH\watchman_unsubscribe(SUB_NAME);
  sleep(3);
  if (HH\watchman_check_sub(SUB_NAME)) {
    print("FAIL\n");
  } else {
    print("PASS\n");
  }

  // Make sure we fail gracefully if we connect to an invalid socket
  print("Checking connecting to a bad socket is graceful\n");
  $saw_exception = false;
  try {
    HH\watchman_query(
      vec[
        'version',
        dict['optional' => vec['relative_root']],
      ],
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

  // Kill Watchman server and check to make sure a subscription gets notified.
  print("Checking detection of dead Watchman server connection\n");
  apc_delete('callback_broken');
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'callback_broken',
    $sock,
  ) |> HH\Asio\join($$);
  $wminst->terminateProcess();
  file_put_contents('should_be_ignored', 'xyz');
  $wait = 0;
  while (!apc_exists('callback_broken') && $wait < 10) {
    sleep(1);
    $wait++;
  }
  if ($wait === 10) {
    print("FAIL (did not see connection error after 10 seconds)\n");
  } else {
    print("PASS (connection error)\n");
  }
  HH\watchman_unsubscribe(SUB_NAME) |> HH\Asio\join($$);
}
<<__EntryPoint>>
function entrypoint_ext_watchman(): void {

  // !!! Please contact devx_www oncall if this breaks. !!!
  //
  // The Watchman extension is a core part of www infrastructure on devservers.

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
