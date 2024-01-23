<?hh
// Copyright 2004-present Facebook. All Rights Reserved.

class CatWrapper {
  private resource $proc;
  private dict<arraykey, resource> $pipes;

  public function __construct() {
    $descriptorspec = dict[
      0 => vec["pipe", "r"],
      1 => vec["pipe", "w"],
    ];
    $__pipes = $this->pipes;
    $this->proc = proc_open("cat", $descriptorspec, inout $__pipes);
    $this->pipes = $__pipes;
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

  public function runAsync(): Awaitable<int> {
    return stream_await($this->pipes[1], STREAM_AWAIT_READ, 10.0);
  }
}

function reschedule($priority = 0) :mixed{
  $queue = RescheduleWaitHandle::QUEUE_DEFAULT;
  return RescheduleWaitHandle::create($queue, $priority);
}

async function correctFrame(Awaitable<int> $awaitable): Awaitable<void> {
  await $awaitable;
}

function backtrace_contains(varray<darray> $bt, string $fn_name): bool {
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
  $wh = $cat->runAsync();

  // try backtracing wait handle, before anything awaits on it
  $bt = HH\Asio\backtrace($wh);
  var_dump(!($bt ?? false));

  $wrapper_frame = async {
    await correctFrame($wh);
  };

  // now something awaits on $wh, but it's not in asio context yet
  $bt = HH\Asio\backtrace($wh);
  var_dump(!($bt ?? false));

  $resv = await HH\Asio\vw(ImmVector {
      $wrapper_frame,
      wrongFrame($cat, $wh),
  });

  // try backtracing wait handle, after it has already finished
  $bt = HH\Asio\backtrace($wh);
  var_dump(!($bt ?? false));

  // try backtracing static wait handle
  $bt = HH\Asio\backtrace(HH\Asio\null());
  var_dump(!($bt ?? false));

  // try backtracing something, that is not a wait handle
  HH\Asio\backtrace($cat);
}


<<__EntryPoint>>
function main_backtrace() :mixed{
HH\Asio\join(testBacktrace());
}
