<?hh

interface Box {
  abstract const type TVal;
  abstract const type TC1;
  abstract const type TC2;
}

// This tests that variance is accumulated properly across type refinements
// We should not report that T1 is redundant.
function good<T0 as shape(...), T1>(
  classname<Box with { type TC1 as T1; type TC2 as T0; type TVal = T0; }> $cls,
  T0 $s,
): T1 {
  throw new Exception("A");
}

// Likewise
function good2<T0 as shape(...), T1>(
  classname<Box with { type TC1 super T1; type TC2 as T0; type TVal = T0; }> $cls,
  T1 $s,
): void {
  throw new Exception("A");
}
