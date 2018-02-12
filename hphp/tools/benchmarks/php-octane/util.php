<?
function QueueRuns($n, $suite) {
  for ($i = 0; $i < $n; $i++) {
    array_push(BenchmarkSuite::$suites, $suite);
  }
}
