<?hh

const SUB_NAME = 'X';
const TEST_FILE_INITIAL = 'initial';
const TEST_FILE_GOT_FRESH = 'got_fresh';
const TEST_FILE_GOT_UPDATE = 'got_update';

function callback_impl(
  string $path,
  string $_query,
  string $_name,
  string $data_json,
  string $_socket_path,
): void {
  // echo "Entered callback\n";
  chdir($path);
  $data = json_decode($data_json, true);
  if ($data['is_fresh_instance'] ?? false) {
    if (!file_exists(TEST_FILE_GOT_FRESH)) {
      file_put_contents(TEST_FILE_GOT_FRESH, $data_json);
    }
  } else if (!file_exists(TEST_FILE_GOT_UPDATE)) {
    file_put_contents(TEST_FILE_GOT_UPDATE, $data_json);
  }
  // echo "Leaving callback\n";
}

async function wait_for(
  string $file,
  WatchmanInstance $wminst,
): Awaitable<void> {
  HH\watchman_check_sub(SUB_NAME);
  // echo "Waiting for $file\n";
  $timeout = 2;
  while ($timeout && !file_exists($file)) {
    await gen_usleep(1000000);
    $timeout--;
  }
  if (file_exists($file)) {
    // echo "Found $file\n";
  } else {
    HH\watchman_check_sub(SUB_NAME);
    // $wminst->dumpLog();
    throw new Exception("timed out on $file");
  }
}

async function gen_usleep(int $usecs): Awaitable<void> {
  await SleepWaitHandle::create($usecs);
}

async function run_tests(): Awaitable<void> {
  require_once realpath(__DIR__.'/../ext_watchman/wminst.inc');
  $tmpdir = tempnam(sys_get_temp_dir(), 'wmt');
  @unlink($tmpdir);
  if (!mkdir($tmpdir)) {
    throw new Exception("failed creating dir '$tmpdir'");
  }
  if (!chdir($tmpdir)) {
    throw new Exception("cannot open dir '$tmpdir'");
  }
  $wminst = null;
  try {
    $wminst = new WatchmanInstance($tmpdir);
    $sock = $wminst->getFullSockName();
    echo "Testing subscription\n";
    file_put_contents(TEST_FILE_INITIAL, '');
    await HH\watchman_subscribe(
      '{"fields": ["name"], "expression": ["exists"]}',
      $wminst->getRepoRoot(),
      SUB_NAME,
      fun('callback'),
      $sock,
    );
    // echo "Subscribed\n";
    await wait_for(TEST_FILE_GOT_FRESH, $wminst);
    await wait_for(TEST_FILE_GOT_UPDATE, $wminst);
    echo "Unsubscribing\n";
    await HH\watchman_unsubscribe(SUB_NAME);
    if (HH\watchman_check_sub(SUB_NAME)) {
      throw new Exception('still reporting subscribed');
    }
    echo "PASS\n";
  } catch (Exception $e) {
    echo "FAIL\n\n";
    echo "Caught: ".$e->getMessage()."\n";
  } finally {
    $wminst?->terminateProcess();
    if (is_dir($tmpdir)) {
      foreach (glob($tmpdir.'/*') as $file) {
        unlink($file);
      }
      rmdir($tmpdir);
    }
  }
}
