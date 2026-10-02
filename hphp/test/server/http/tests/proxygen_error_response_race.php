<?hh
/*
 * The situation under test: under heavy contention a queue-timeout 503 is
 * committed by the request thread but is still only queued on the worker when
 * that same request's ingress timeout fires on the EventBase. The error path
 * then tries to replace a response that is already committed -- two threads
 * answering one request.
 *
 * A blocker holds the single request thread just under the ingress timeout, so
 * the requests queued behind it all come due at once when it releases. They are
 * POSTs whose Content-Length the client never fulfils; curl will not do that,
 * hence the raw /dev/tcp clients.
 *
 * Why so many connections: the window between "503 queued on the worker" and
 * "worker drains the queue" is sub-millisecond, so any single collision is
 * unlikely. It becomes reliable only when a large number of idle timers expire
 * within the same 10ms timer-wheel tick while the worker is draining. The knee
 * is sharp -- 0/5 runs detect at 1000 sockets, 3/4 at 1520, 5/5 at 2000 and
 * 3000.
 */

const int NUM_CHILDREN = 40;
const int PER_CHILD = 50;   // 2000 stalled POSTs
const int BLOCK_MS = 1900;
const int NUM_ROUNDS = 8;

<<__EntryPoint>>
function main(): void {
  require_once('test_base.inc');
  init();
  runTest(
    function ($serverPort) {
      $dir = sys_get_temp_dir().'/proxygen_race_'.posix_getpid();
      mkdir($dir, 0777, true);
      $totalDropped = 0;
      $alive = true;
      // A few rounds: each one is an independent chance at the collision.
      for ($round = 0; $round < NUM_ROUNDS && $alive; $round++) {
      $blastAt = microtime(true) + 1.5;

      $script = __DIR__.'/proxygen_error_response_race_clients.sh';
      $cmd = 'bash '.escapeshellarg($script).' '.(int)$serverPort.' '.
             NUM_CHILDREN.' '.PER_CHILD.' '.
             sprintf('%.0f', $blastAt * 1000000000).' '.escapeshellarg($dir);
      $pipes = dict[];
      $burst = proc_open($cmd, dict[], inout $pipes);

      // Hold the request thread, releasing it just before the timers expire.
      $sleep = (int)(($blastAt - microtime(true) - 0.3) * 1000000);
      if ($sleep > 0) usleep($sleep);
      $errno = null;
      $errstr = null;
      $blocker = stream_socket_client(
        "localhost:".$serverPort, inout $errno, inout $errstr, 10.0);
      fwrite($blocker,
        "GET /timeout_race_slow.php?ms=".BLOCK_MS." HTTP/1.1\r\n".
        "Host: localhost\r\n\r\n");

      proc_close($burst);
      fclose($blocker);

      $answered = 0;
      $dropped = 0;
      for ($i = 1; $i <= NUM_CHILDREN; $i++) {
        $f = $dir.'/'.$i;
        if (!file_exists($f)) continue;
        $parts = explode(' ', trim((string)file_get_contents($f)));
        if (count($parts) === 2) {
          $answered += (int)$parts[0];
          $dropped += (int)$parts[1];
        }
      }

      $totalDropped += $dropped;
      // Retry: a crashed server never answers, but a starved one can miss a
      // single probe, and treating that as death would fail the test on a
      // small machine for the wrong reason.
      $alive = false;
      for ($try = 0; $try < 20; $try++) {
        $after = request('localhost', $serverPort, 'hello.php');
        if ($after is string && strpos($after, 'Hello, World!') === 0) {
          $alive = true;
          break;
        }
        usleep(500000);
      }
      }

      var_dump($alive);
      // Clients are answered rather than reset. A bound, not an equality: a
      // handful out of ~16k connections are lost to connection setup under a
      // burst this size.
      $total = NUM_ROUNDS * NUM_CHILDREN * PER_CHILD;
      var_dump($totalDropped * 100 < $total);
    },
    '-vServer.ThreadCount=1'.
    ' -vServer.IOThreadCount=1'.
    ' -vServer.RequestBodyReadLimit=8192'.
    ' -vServer.RequestTimeoutSeconds=1'.
    ' -vServer.ConnectionTimeoutSeconds=2'.
    ' -vServer.Backlog=4096',
  );
}
