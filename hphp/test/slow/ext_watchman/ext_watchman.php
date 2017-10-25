<?hh

# !!! Please contact devx_www oncall if this breaks. !!!
#
# The Watchman extension is a core part of www infrastructure on devservers.

require_once 'constants.php';
require_once 'callback.php';

// Borrowed and stripped down from Watchman project test infrastructure.
class WatchmanInstance {
  private $proc;
  private $logfile;
  private $sockname;
  private $config_file;
  private $repo_root;
  private $run_details = '';
  const TIMEOUT = 20;
  private function tempfile() {
    $temp = tempnam(sys_get_temp_dir(), 'wat');
    return $temp;
  }
  function __construct($tmpdir) {
    // Not atomic but PHP doesn't support this. Should be fine.
    $this->repo_root = $tmpdir;
    $this->logfile = $this->tempfile();
    $this->sockname = $this->tempfile();
    $this->config_file = $this->tempfile();
    $config_json = '{}';
    file_put_contents($this->config_file, $config_json);
    $this->start();
  }
  function getFullSockName() {
    return $this->sockname . '.sock';
  }
  function getRepoRoot() {
    return $this->repo_root;
  }
  private function start() {
    $cmd = "watchman --foreground --sockname=%s --logfile=%s " .
            "--statefile=%s.state --log-level=2 --pidfile=%s";
    putenv("WATCHMAN_CONFIG_FILE=".$this->config_file);
    $cmd = sprintf(
      $cmd,
      $this->getFullSockName(),
      $this->logfile,
      $this->logfile,
      $this->tempfile(),
    );
    $pipes = array();
    $this->run_details = "Ran command: $cmd, in ".$this->repo_root;
    $this->proc = proc_open($cmd, array(
      0 => array('file', '/dev/null', 'r'),
      1 => array('file', $this->logfile, 'a'),
      2 => array('file', $this->logfile, 'a'),
    ), $pipes, $this->repo_root);
    if (!$this->proc) {
      throw new Exception("Failed to spawn $cmd");
    }
    try {
      $this->openSock();
    } catch(Exception $e) {
      $this->dumpLog();
      throw $e;
    }
  }
  public function dumpLog() {
    print("Dumping log:\n");
    system("cat $this->logfile");
    print($this->run_details."\n");
  }
  private function openSock() {
    $sockname = $this->getFullSockName();
    $deadline = time() + 5;
    do {
      if (!file_exists($sockname)) {
        usleep(30000);
      }
      $this->sock = @fsockopen('unix://' . $sockname);
      if ($this->sock) {
        break;
      }
    } while (time() <= $deadline);
    if (!$this->sock) {
      throw new Exception("Failed to talk to watchman on $sockname");
    }
    stream_set_timeout($this->sock, self::TIMEOUT);
  }
  private function waitForTerminate($timeout) {
    if (!$this->proc) {
      return false;
    }
    $deadline = time() + $timeout;
    do {
      $st = proc_get_status($this->proc);
      if (!$st['running']) {
        return $st;
      }
      usleep(30000);
    } while (time() <= $deadline);
    return $st;
  }
  function terminateProcess() {
    if (!$this->proc) {
      return;
    }
    proc_terminate($this->proc);
    $st = $this->waitForTerminate(self::TIMEOUT);
    if ($st['running']) {
      echo "Didn't stop, sending bigger signal\n";
      proc_terminate($this->proc, 9);
      $st = $this->waitForTerminate(5);
      $this->proc = null;
    }
  }
  function __destruct() {
    $this->terminateProcess();
  }
}

function waitFor(string $filename, WatchmanInstance $wminst): void {
  print("Waiting for $filename\n");
  $timeout = 5;
  while($timeout && !file_exists(TEST_FILE_GOT_FRESH)) {
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

function test_core(string $tmpdir): void {
  if ((int)HH\ext_watchman_version()) {
    print "PASSED version get test\n";
  } else {
    print "FAILED version get test\n";
  }

  $wminst = new WatchmanInstance($tmpdir);
  $sock = $wminst->getFullSockName();
  // Test one-shot
  print("Testing one-shot\n");
  $version_h =
    HH\watchman_run('["version", {"optional":["relative_root"]}]', $sock);
  $version = json_decode(HH\asio\join($version_h), true);
  if (
    !isset('capabilities', $version) ||
    $version['capabilities'] !== array('relative_root' => true)
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
  ) |> HH\asio\join($$);
  print("Subscribed\n");
  waitFor(TEST_FILE_GOT_FRESH, $wminst);
  waitFor(TEST_FILE_GOT_UPDATE, $wminst);
  print("Unsubscribing\n");
  $str = HH\watchman_unsubscribe(SUB_NAME) |> HH\asio\join($$);
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
  ) |> HH\asio\join($$);
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
  HH\watchman_unsubscribe(SUB_NAME) |> HH\asio\join($$);

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
  ) |> HH\asio\join($$);
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
  HH\watchman_unsubscribe(SUB_NAME) |> HH\asio\join($$);

  print("Stress test\n");
  apc_store('stress_counter', 0);
  fb_call_user_func_array_async(
    __DIR__.'/callback.php',
    'callback_checksub',
    varray[SUB_NAME],
  );
  // Success for this test is just not crashing HHVM
  $subscribed = false;
  $subscribe_c = 0;
  $unsubscribe_c = 0;
  $touch_c = 0;
  $last_subscribe = null;
  $last_unsubscribe = null;
  $exception_count = array(0, 0, 0);
  $op_count = array(0, 0, 0);
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
      $exception_count = array(0, 0, 0);
      $op_count = array(0, 0, 0);
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
          file_put_contents($wminst->getRepoRoot() + '/afile', 'X');
          break;
      }
    } catch (Exception $e) {
      $exception_count[$op]++;
    }
    usleep(rand() % 1000);
  }
  try {
    if ($last_subscribe) {
      HH\asio\join($last_subscribe);
    }
    if ($last_unsubscribe) {
      HH\asio\join($last_unsubscribe);
    }
  } catch(Exception $_) { }
  if (HH\watchman_check_sub(SUB_NAME)) {
    HH\asio\join(HH\watchman_unsubscribe(SUB_NAME));
  }
  $hit_c = apc_fetch('stress_counter');
  if (!$subscribe_c || !$unsubscribe_c || !$touch_c || !$hit_c) {
    print("\nFAIL ($subscribe_c, $unsubscribe_c, $touch_c, $hit_c)\n");
  } else {
    print("\nPASS ($subscribe_c, $unsubscribe_c, $touch_c, $hit_c)\n");
  }
  apc_delete('stress_counter');  // Stops async callback_checksub()
  sleep(1);

  # Sync testing is based on the fact there will immediately be an event to
  # process as Watchman will send us an "initial" update straight away. There
  # should then be no further updates.
  print("Testing syncing\n");
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'callback_sync',
    $sock,
  ) |> HH\asio\join($$);
  print("Subscribed, sleeping for 1 second...\n");
  sleep(1);
  print("Sync expecting timeout after 100ms...\n");
  $synced = HH\watchman_sync_sub(SUB_NAME, 100) |> HH\asio\join($$);
  if ($synced) {
    print("FAIL\n");
  } else {
    print("PASS\n");
  }
  print("Sync expecting no timeout...\n");
  if (HH\asio\join(HH\watchman_sync_sub(SUB_NAME, 10000))) {
    print("PASS\n");
  } else {
    print("FAIL\n");
  }
  print("Sync expecting immediate return...\n");
  $synced = HH\watchman_sync_sub(SUB_NAME, 1) |> HH\asio\join($$);
  if ($synced) {
    print("PASS\n");
  } else {
    print("FAIL\n");
  }
  print("Unsubscribing\n");
  $str = HH\watchman_unsubscribe(SUB_NAME) |> HH\asio\join($$);

  print("Check catching exception raised in a subscription callback\n");
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'callback_exception',
    $sock,
  )|> HH\asio\join($$);
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
  HH\watchman_unsubscribe(SUB_NAME) |> HH\asio\join($$);

  # Check we can unsubscribe while a callback is in progress. The easiest way
  # synthesize this event is to unsubscribe in a callback itself.
  print("Check unsubscribing while callback in progress\n");
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'callback_unsubscribe',
    $sock,
  )|> HH\asio\join($$);
  HH\watchman_unsubscribe(SUB_NAME);
  sleep(3);
  if (HH\watchman_check_sub(SUB_NAME)) {
    print("FAIL\n");
  } else {
    print("PASS\n");
  }

  # Make sure we fail gracefully if we connect to an invalid socket
  print("Checking connecting to a bad socket is graceful\n");
  $saw_exception = false;
  try {
    HH\watchman_run('["version", {"optional":["relative_root"]}]', "garbage")
      |> HH\asio\join($$);
  } catch(Exception $e) {
    print('Got exception: '.$e->getMessage()."\n");
    $saw_exception = true;
  }
  if ($saw_exception) {
    print("PASS\n");
  } else {
    print("FAIL\n");
  }

  # Kill Watchman server and check to make sure a subscription gets notified.
  print("Checking detection of dead Watchman server connection\n");
  apc_delete('callback_broken');
  HH\watchman_subscribe(
    '{"fields": ["name"], "expression": ["exists"]}',
    $wminst->getRepoRoot(),
    SUB_NAME,
    'callback_broken',
    $sock,
  ) |> HH\asio\join($$);
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
  HH\watchman_unsubscribe(SUB_NAME) |> HH\asio\join($$);
}

$tmpdir = tempnam(sys_get_temp_dir(), 'wmt');
@unlink($tmpdir);
if (!mkdir($tmpdir)) {
  throw new Exception("FAIL failed creating dir '$tmpdir'\n");
}
try {
  if (!chdir($tmpdir)) {
    throw new Exception("FAIL (creating temporary directory)\n");
  }
  test_core($tmpdir);
} finally {
  apc_delete('stress_counter');  // Stops async callback_checksub() if running
  if (is_dir($tmpdir)) {
    foreach (glob($tmpdir.'/*') as $file) {
      unlink($file);
    }
    rmdir($tmpdir);
  }
}
