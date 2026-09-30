<?hh

// Regression test for cmpWillThrow, which answered through Type::couldBe and
// so treated a marked array and an unmarked one as types that can never meet.
// HHBBC concluded the relational comparison below could not return, marked
// what follows unreachable, and planted a StaticAnalysisError there; the
// comparison returns normally, so the poisoned instruction was reached.
//
// Only reachable under -r, since it takes HHBBC to plant it.  Before the fix
// the program died on the trap that instruction lowers to, leaving no output
// at all rather than a failed comparison.
//
// Only two things matter, each checked by substitution: the comparison has to
// be relational -- with == HHBBC draws no such conclusion -- and one side has
// to come from array_mark_legacy, since comparing $a against itself is fine.
// The vec's length and element type, and the second argument to
// array_mark_legacy, are all irrelevant.

<<__EntryPoint>>
function main(): void {
  $a = vec[1];
  $b = \HH\array_mark_legacy($a, true);
  \var_dump($a > $b);
  \var_dump($a == $b);
  echo "done\n";
}
