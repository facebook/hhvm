<?hh

// FAILS under `-r --retranslate-all 2 --async-jit-profile`: open bug, not yet
// diagnosed.
//
// Appending to a dict whose largest key is INT64_MAX cannot pick a next key,
// so it warns and adds nothing. Writing the base back afterwards trips a
// refcount assertion:
//
//   member-operations.h:1231: void HPHP::arraySetUpdateBase(ArrayData *, tv_lval):
//   assertion `newData->hasExactlyOneRef()' failed
//
// All three of those flags are needed, each checked by substitution. Repo mode
// and retranslate-all put the Optimize tier on HHBBC-optimized bytecode, and
// neither alone is enough; async JIT profiling is on by default but the test
// runner turns it off for retranslate-all unless asked, and with it off this
// passes.
//
// The program itself is minimal in the same sense: with any key other than
// INT64_MAX the append succeeds and nothing goes wrong, and without the loop
// the function never gets hot enough to be retranslated.

<<__EntryPoint>>
function main(): void {
  $n = 0;
  for ($i = 0; $i < 50; ++$i) {
    $d = dict[9223372036854775807 => false];
    $d[] = true;
    $n += \count($d);
  }
  echo $n, "\n";
}
