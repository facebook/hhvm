<?hh

class C { public function __construct(public $x)[] {} }

// Returns a permutation of the set {0, 1, ... 2^n - 1} that is Hamiltonion
// in the hypercube on n vertices.
function path(int $n): vec<int> {
  if ($n === 0) return vec[0];
  $prev = path($n - 1);
  $size = count($prev);
  for ($i = 0; $i < $size; $i++) {
    $prev[] = $size | $prev[$size - $i - 1];
  }
  return $prev;
}

// Comprehensive test of dict insertion and deletion. Runtime: O(n * 2^n).
function test(int $n) :mixed{
  print("\n==============================================================\n");
  print("Running test($n):\n");
  $path = path($n);
  $size = count($path);

  $dict = dict[];
  for ($i = 0; $i < $size; $i++) {
    $dict[$i] = new C($i);
  }

  $target = 1;
  foreach ($path as $i => $item) {
    unset($dict[$item]);
    if ($i + 1 === $target) {
      // Compute the sum of the elements by doing lookups.
      $sum = 0;
      for ($j = 0; $j < $size; $j++) {
        $sum += idx($dict, $j)?->x ?? 0;
      }
      var_dump($sum);
      // Recompute the sum by iterating.
      $iter = 0;
      foreach ($dict as $value) {
        $iter += $value->x;
      }
      if ($iter !== $sum) {
        throw new Error('Unexpected sum from iteration!');
      }
      $target = $target * 3;
    }
  }
}

<<__EntryPoint>>
function main() :mixed{
  // NOTE: The 18 here is the minimum size at which we'll hit MonotypeDict's
  // maximum tombstone count limit, which is what we're trying to test.
  for ($i = 0; $i < 18; $i++) {
    test($i);
  }
}
