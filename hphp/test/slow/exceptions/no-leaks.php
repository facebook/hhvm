<?hh

class FooException extends Exception {
  public string $a;
  function __construct() {
    $str = ' ';
    for ($i = 0; $i < 20; ++$i) $str.= $str;
    $this->a = $str;
  }
}

function f() :mixed{
  throw new FooException();
}

<<__NEVER_INLINE>>
function run() :mixed{
  $delta = 0;
  for ($i = 0; $i < 10; $i++) {
    $start = memory_get_usage(true);
    try { f(); } catch (Exception $e) { $e = null; }
    $delta += (memory_get_usage(true) - $start);
  }
  return $delta;
}


<<__EntryPoint>>
function main() :mixed{
  run(); run(); run(); // ignore
  $delta1 = abs(run());
  $delta2 = abs(run());
  if ($delta1 + $delta2 > $delta1 * 1.1 + 100000) {
    echo "Memory leak!!\n";
  } else {
    echo "ok!\n";
  }
}
