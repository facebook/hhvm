<?hh

/*
 * The JIT expands `X ** C`, for a constant non-negative integer C, into a
 * chain of overflow-checking multiplies, and falls back to computing the
 * whole thing in floating point when one of them overflows.
 *
 * ARM has no hardware overflow flag for multiply, so the emitter derives one
 * from the upper 64 bits of the product.  Reading the operands for that has
 * to happen before the multiply writes its destination, which the register
 * allocator is free to give the same register as one of the operands.
 *
 * A spurious overflow does not show up in the value -- the float carries the
 * same number -- only in the type, so every result is compared with === to
 * one computed with a laundered exponent, where no expansion applies and the
 * generic helper runs.  The interpreter has no expansion and always agrees.
 */

function opaque(int $v): int {
  __hhvm_intrinsics\launder_value_inout(inout $v);
  return $v;
}

function pow2(int $x): num  { return $x ** 2; }
function pow3(int $x): num  { return $x ** 3; }
function pow7(int $x): num  { return $x ** 7; }
function pow27(int $x): num { return $x ** 27; }
function pow31(int $x): num { return $x ** 31; }
function pow32(int $x): num { return $x ** 32; }
function pow62(int $x): num { return $x ** 62; }
function pow63(int $x): num { return $x ** 63; }

function powOpaque(int $x, int $e): num { return $x ** opaque($e); }

<<__EntryPoint>>
function main(): void {
  $inputs = vec[0, 1, -1, 2, -2, 3, -3, 5, -5, PHP_INT_MAX, PHP_INT_MIN];
  $exps = vec[2, 3, 7, 27, 31, 32, 62, 63];

  $bad = 0;
  // Enough calls to get these translated, and retranslated to Optimize under
  // --retranslate-all.
  for ($i = 0; $i < 2000; ++$i) {
    foreach ($inputs as $x) {
      $x = opaque($x);
      $expanded = vec[
        pow2($x),
        pow3($x),
        pow7($x),
        pow27($x),
        pow31($x),
        pow32($x),
        pow62($x),
        pow63($x),
      ];
      foreach ($exps as $j => $e) {
        $want = powOpaque($x, $e);
        if ($expanded[$j] === $want) continue;
        ++$bad;
        if ($bad <= 10) {
          // The type is the interesting half: a spurious overflow keeps the
          // value and only changes int to float.
          printf("MISMATCH %d ** %d: expanded %s %s, opaque %s %s\n",
                 $x, $e,
                 gettype($expanded[$j]), var_export($expanded[$j], true),
                 gettype($want), var_export($want, true));
        }
      }
    }
  }
  printf("%d mismatches\n", $bad);

  // Two rows spelled out, so that the expected semantics are visible in the
  // expect file: the result stays an int, of either sign, until a multiply
  // genuinely overflows, and only then becomes a float.
  foreach (vec[2, -2] as $base) {
    $b = opaque($base);
    foreach ($exps as $e) {
      printf("%d ** %d = %s\n", $base, $e, var_export(powOpaque($b, $e), true));
    }
  }
}
