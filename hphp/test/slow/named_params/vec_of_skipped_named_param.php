<?hh
<<file: __EnableUnstableFeatures('named_parameters')>>

// HHBBC asserts building the repo:
//
//   type-system.cpp:3132: Type::checkInvariants():
//   assertion `!v.is(BBottom)' failed
//
// so the whole repo build dies, not just this unit.  Found by
// hphp/tools/hackgen; see FUZZER.md.  Only reachable under -r, since it takes
// HHBBC, and the program itself is correct -- the .expect is what it prints
// without the repo build.
//
// Every ingredient was checked by substitution, and the set is oddly narrow:
//
//   - the call has to skip a *middle* named parameter while passing a later
//     one.  Passing a1 is fine, and so is a two-parameter version that drops
//     only the last one.
//   - the body has to put the skipped parameter in a `vec`.  Copying it to a
//     local, returning it, or using `keyset` instead are all fine, and so is a
//     vec of one of the parameters that was passed.
//   - the call has to be in a function of its own.  The same call written
//     directly in the __EntryPoint does not do it, nor does returning the vec
//     straight out of f1 rather than storing it first.
//
// Reified generics, the class the original had, and the value of the defaults
// all turned out to be irrelevant.

function caller(): void {
  $r = f1(a0=true, a2=1.0);
  \var_dump($r);
}

function f1(named bool $a0, named int $a1 = 3, named float $a2 = 2.0): bool {
  $v = vec[$a1];
  \var_dump($v);
  return $a0;
}

<<__EntryPoint>>
function main(): void {
  caller();
}
