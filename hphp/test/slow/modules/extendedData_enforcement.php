<?hh

module MLT_A;

<<file: __EnableUnstableFeatures('module_level_traits')>>

/** This doc string enforces the extended data section to be allocated
 * in the Func object of test.
 */
<<__NeverInline>>
internal function test(): int {
  return 42;
}

<<__EntryPoint>>
function main(): void {
  echo test()."\n";
}
