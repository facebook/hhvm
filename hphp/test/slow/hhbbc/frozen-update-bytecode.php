<?hh

// Test for a bug where update_bytecode classified a block-size-changing
// rewrite as Changed instead of ChangedAnalyze, skipping re-analysis.
//
// The ternary `$count = $found ? $count + 1 : $count` generates a
// false-branch block that pushes $count onto the stack (CGetQuietL),
// followed by a join block that pops it back (PopL $count). During
// speculation, the PopL reduces to PopC (same-value round-trip), making
// the block effect-free. The successful speculation rewrites the
// false-branch block by appending a PopC, changing its bytecode count.
//
// Without re-analysis, the frozen pass sees the rewritten block (which
// no longer pushes $count for the join), changing speculation behavior
// at the join block and enabling a new JmpZ reduction on $found that
// wasn't present before — triggering the frozen assertion.

function test(vec<vec<int>> $items): void {
  $count = 0;
  foreach ($items as $outer) {
    $found = idx($outer, 0) is nonnull;
    $count = ($found) ? $count + 1 : $count;
    if ($found) {
      echo "y\n";
    }
  }
}

<<__EntryPoint>>
function main(): void {
  test(vec[]);
  echo "done\n";
}
