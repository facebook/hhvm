<?hh

// Exercises Eval.AsyncJitWaitForTranslate: with the async JIT enabled, the
// request thread enqueues each translation/prologue onto the worker, blocks
// until it is produced off the request thread, then runs it. Output must be
// identical to running with the JIT synchronously, just produced off-thread.

function fib(int $n): int {
  if ($n < 2) return $n;
  return fib($n - 1) + fib($n - 2);
}

function work(int $iters): int {
  $sum = 0;
  for ($i = 0; $i < $iters; $i++) {
    $sum += fib($i % 20);
  }
  return $sum;
}

<<__EntryPoint>>
function main(): void {
  // Loop enough to force prologue + region translations to be enqueued.
  for ($i = 0; $i < 5; $i++) {
    echo work(1000) . "\n";
  }
}
