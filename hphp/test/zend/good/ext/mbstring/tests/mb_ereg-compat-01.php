<?hh
<<__EntryPoint>> function main(): void {
  /* (counterpart: ext/standard/tests/reg/004.phpt) */
  $a="This is a nice and simple string";
  $regs = null;
  if (mb_ereg(".*nice and simple.*",$a, inout $regs)) {
    echo "ok\n";
  }
  if (!mb_ereg(".*doesn't exist.*",$a, inout $regs)) {
    echo "ok\n";
  }
}
