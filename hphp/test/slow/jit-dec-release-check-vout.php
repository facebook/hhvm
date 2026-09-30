<?hh

// Regression test for two bugs in cgDecReleaseCheck, both of which emitted
// into the wrong Vout and tripped
//
//   vasm-gen.cpp:35: Vout::operator<<(const Vinstr&): assertion `!closed()'
//
// ifThenElseRefCountedType hands its then-block a Vout for the block it just
// branched into, and cgDecReleaseCheck ignored it and used vmain, which that
// branch had already closed.  Fixing that exposed the second: for a type that
// is definitely uncounted no branch is emitted at all, and the else body was
// going to the cold Vout rather than the current one, leaving the block with
// no terminator.
//
// Only reachable under `-r --retranslate-all 2`.  refcount-opts rewrites a
// DecRef into DecReleaseCheck once profiling says the site releases arrays of
// uncounted elements, which needs an Optimize translation, and the assertion
// then fires while the inliner lowers the unit to estimate its cost.
//
// The append is what produces the DecRef: appending to a vec parameter copies
// it, and the original is released.  chk() is not decoration -- it is what
// gives that DecRef site its profile.  Dropping its string branch, or the
// inner loop that calls f0 four times per iteration, stops it reproducing.
//
// Reduced from a hackgen fuzzer program; reproduced five runs in six before
// the fix on an otherwise idle machine, and rather less than that on a busy
// one.

abstract final class S {
  public static string $trace = '';
}
function chk(mixed $x, int $d = 0): string {
  if ($x is string) {
    return 'S'.(string)\strlen($x);
  }
  if (\HH\is_vec($x) || \HH\is_dict($x) || \HH\is_keyset($x)) {
    foreach ($x as $k => $v) {
      $s .= chk($k, $d + 1).'=>'.chk($v, $d + 1).',';
    }
  }
}
function sink(mixed $x): void {
  S::$trace .= chk($x).';';
}
async function f0(vec<string> $v): Awaitable<keyset<string>> {
  $empty = "";
  $v[] = $empty;
  $elem = $v[1];
  sink($empty);
  $str = "1e3";
  $ks = keyset[$str, $elem, $elem];
  return $ks;
}
<<__EntryPoint>>
async function main(): Awaitable<void> {
  for ($i = 0; $i < 50; ++$i) {
    for ($j = 0; $j < 4; ++$j) {
      $r = '';
      try {
        $r = chk(await f0(vec["0", "0"]));
      } catch (\Throwable $e) {
      }
      if ($i === 0) {
        echo (string)$i, ',', (string)$j, ': ', S::$trace, '|', $r, "\n";
      }
    }
  }
}
