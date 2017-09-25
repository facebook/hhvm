<?hh // strict
// Copyright 2004-present Facebook. All Rights Reserved.

class CatWrapper {
  private resource $proc;
  private array $pipes;

  public function __construct() {
    $descriptorspec = array(
      0 => array("pipe", "r"),
      1 => array("pipe", "w"),
    );
    $this->proc = proc_open("cat", $descriptorspec, $this->pipes);
    if (!is_resource($this->proc) ||
        !is_resource($this->pipes[0]) ||
        !is_resource($this->pipes[0])) {
      throw new Exception("proc_open failed!");
    }
  }

  public function close(): void {
    fclose($this->pipes[0]);
    fclose($this->pipes[1]);
    proc_close($this->proc);
  }

  public function complete(): void {
    fwrite($this->pipes[0], "done\n");
    fflush($this->pipes[0]);
  }

  public function getWaitHandle(): Awaitable<int> {
    return stream_await($this->pipes[1], STREAM_AWAIT_READ, 10.0);
  }
}

function reschedule($priority = 0) {
  $queue = RescheduleWaitHandle::QUEUE_DEFAULT;
  return RescheduleWaitHandle::create($queue, $priority);
}

async function correctFrame(Awaitable<int> $awaitable): Awaitable<void> {
  await $awaitable;
}

function backtrace_contains(array $bt, string $fn_name): bool {
  foreach ($bt as $frame) {
    if (idx($frame, 'function') === $fn_name) {
      return true;
    }
  }
  return false;
}

async function wrongFrame(
  CatWrapper $cat,
  Awaitable<int> $wh
): Awaitable<void> {
  // stop eager execution
  await reschedule();

  // backtrace awaitable, make sure, we get correct backtrace
  $bt = HH\Asio\backtrace($wh);
  var_dump(backtrace_contains($bt, 'correctFrame') &&
           !backtrace_contains($bt, 'wrongFrame'));

  // backtrace ourselves, make sure it still works
  $bt = debug_backtrace();
  var_dump(!backtrace_contains($bt, 'correctFrame') &&
           backtrace_contains($bt, 'wrongFrame'));

  // backtrace ourselves using HH\Asio\backtrace
  $bt = HH\Asio\backtrace(asio_get_running());
  var_dump(!backtrace_contains($bt, 'correctFrame') &&
           backtrace_contains($bt, 'wrongFrame'));

  $cat->complete();
}

async function testBacktrace(): Awaitable<void> {
  $cat = new CatWrapper();
  $wh = $cat->getWaitHandle();

  // try backtracing wait handle, before anything awaits on it
  $bt = HH\Asio\backtrace($wh);
  var_dump(empty($bt));

  $wrapper_frame = async {
    await correctFrame($wh);
  };

  // now something awaits on $wh, but it's not in asio context yet
  $bt = HH\Asio\backtrace($wh);
  var_dump(empty($bt));

  $resv = await HH\Asio\vw(ImmVector {
      $wrapper_frame,
      wrongFrame($cat, $wh),
  });

  // try backtracing wait handle, after it has already finished
  $bt = HH\Asio\backtrace($wh);
  var_dump(empty($bt));

  // try backtracing static wait handle
  $bt = HH\Asio\backtrace(HH\Asio\null());
  var_dump(empty($bt));

  // try backtracing something, that is not a wait handle
  try {
    $bt = HH\Asio\backtrace($cat);
    var_dump(false);
  } catch (InvalidArgumentException $e) {
    var_dump(true);
  }
}

HH\Asio\join(testBacktrace());
