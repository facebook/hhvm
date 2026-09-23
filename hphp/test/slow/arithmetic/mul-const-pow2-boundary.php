<?hh

/*
 * The JIT's simplifyMulInt() rewrites a multiply by a constant into shifts:
 *
 *   X * 2^C        ->  X << C
 *   X * (2^C + 1)  ->  (X << C) + X
 *   X * (2^C - 1)  ->  (X << C) - X
 *
 * The last two decide whether they apply by testing rhs - 1 and rhs + 1 for
 * being a power of two, and those signed-overflow when the constant is
 * PHP_INT_MIN or PHP_INT_MAX.
 *
 * Whatever the constant, the rewritten form has to produce the same value as
 * an ordinary multiply, so every product is checked against one computed with
 * both operands laundered, where no constant rewrite applies.  The
 * interpreter has no such rewrite and always agrees.
 */

function opaque(int $v): int {
  __hhvm_intrinsics\launder_value_inout(inout $v);
  return $v;
}

function mulPow2Minus1(int $x): int { return $x * 4611686018427387903; }
function mulPow2(int $x): int       { return $x * 4611686018427387904; }
function mulPow2Plus1(int $x): int  { return $x * 4611686018427387905; }
function mulIntMax(int $x): int     { return $x * PHP_INT_MAX; }
function mulIntMin(int $x): int     { return $x * PHP_INT_MIN; }

function mulOpaque(int $x, int $m): int { return $x * opaque($m); }

<<__EntryPoint>>
function main(): void {
  $inputs = vec[0, 1, 3, -1, PHP_INT_MAX, PHP_INT_MIN];
  $consts = vec[
    4611686018427387903, // 2^62 - 1
    4611686018427387904, // 2^62
    4611686018427387905, // 2^62 + 1
    PHP_INT_MAX,
    PHP_INT_MIN,
  ];

  $bad = 0;
  // Enough calls to get these translated, and retranslated to Optimize under
  // --retranslate-all.
  for ($i = 0; $i < 2000; ++$i) {
    foreach ($inputs as $x) {
      $x = opaque($x);
      $folded = vec[
        mulPow2Minus1($x),
        mulPow2($x),
        mulPow2Plus1($x),
        mulIntMax($x),
        mulIntMin($x),
      ];
      foreach ($consts as $j => $m) {
        $want = mulOpaque($x, $m);
        if ($folded[$j] === $want) continue;
        ++$bad;
        if ($bad <= 10) {
          printf("MISMATCH %d * %d: folded %d, opaque %d\n",
                 $x, $m, $folded[$j], $want);
        }
      }
    }
  }
  printf("%d mismatches\n", $bad);

  // One row spelled out, so that the expected semantics are visible in the
  // expect file: multiplying ints wraps, it does not promote to float.
  $three = opaque(3);
  foreach ($consts as $m) {
    printf("3 * %d = %d\n", $m, mulOpaque($three, $m));
  }
}
