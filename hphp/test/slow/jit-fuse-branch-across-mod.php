<?hh

// Branch fusion repoints the `if` below at the comparison's own flags and drops
// the intervening testb. It runs before x64 lowering, which turns the modulo's
// `srem` into an `idiv` -- an instruction that clobbers the flags -- so the
// branch was left reading whatever the division produced. Debug builds trip
// vasm-check's "no two status-flag lifetimes overlap" assertion; optimized
// builds just take the wrong branch.
//
// Both locals are load-bearing: without $lt no setcc is materialized for fusion
// to match, and the modulo has to sit between the comparison and the branch.

function f(int $z): int {
  $lt = 5 > $z;
  $m = $z % 3;
  if ($lt) {
    return $m;
  }
  return 100;
}

<<__EntryPoint>>
function main(): void {
  // 5 > 4 holds every time, so this is 50 * (4 % 3), never 50 * 100.
  $t = 0;
  for ($i = 0; $i < 50; ++$i) {
    $t += f(4);
  }
  echo $t, "\n";
}
