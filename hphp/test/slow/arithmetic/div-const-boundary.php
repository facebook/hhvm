<?hh

/*
 * The JIT's simplifyDivInt() folds a division by a constant:
 *
 *   X / 2^n   ->  X >> n
 *   X / -2^n  ->  -(X >> n)
 *   C / D     ->  the folded quotient
 *
 * Two things go wrong at the int boundaries.  Taking the magnitude of a
 * negative divisor negates it, which signed-overflows at PHP_INT_MIN.  And
 * folding two constants used to additionally require that the dividend not be
 * PHP_INT_MIN, even though emitDiv() only sends PHP_INT_MIN down the float
 * path when the divisor is -1, so every other PHP_INT_MIN division arrives
 * here.
 *
 * Only exact divisions reach DivInt at all: an inexact one produces a float
 * through another opcode, which is why every quotient below is an int.
 */

function opaque(int $v): int {
  __hhvm_intrinsics\launder_value_inout(inout $v);
  return $v;
}

// Constant divisor, opaque dividend: exercises the shift rewrites.
function divPow2(int $x): int    { return $x / 4611686018427387904; }  // 2^62
function divNegPow2(int $x): int { return $x / -4611686018427387904; }
function divIntMin(int $x): int  { return $x / PHP_INT_MIN; }

// Both operands constant: exercises the fold.
function constDivs(): vec<int> {
  $min = PHP_INT_MIN;
  $two = 2;
  $four = 4;
  $negTwo = -2;
  return vec[$min / $two, $min / $four, $min / $negTwo, $min / $min];
}

<<__EntryPoint>>
function main(): void {
  $byPow2 = vec[0, 4611686018427387904, -4611686018427387904, PHP_INT_MIN];
  // Only these two divide exactly by PHP_INT_MIN.
  $byIntMin = vec[0, PHP_INT_MIN];

  $pos = vec[];
  $neg = vec[];
  $min = vec[];
  $folded = vec[];
  // Enough calls to get these translated, and retranslated to Optimize under
  // --retranslate-all.
  for ($i = 0; $i < 2000; ++$i) {
    $pos = vec[];
    $neg = vec[];
    $min = vec[];
    foreach ($byPow2 as $x) {
      $x = opaque($x);
      $pos[] = divPow2($x);
      $neg[] = divNegPow2($x);
    }
    foreach ($byIntMin as $x) {
      $min[] = divIntMin(opaque($x));
    }
    $folded = constDivs();
  }

  printf("x / 2^62:  %s\n", \implode(" ", $pos));
  printf("x / -2^62: %s\n", \implode(" ", $neg));
  printf("x / min:   %s\n", \implode(" ", $min));
  printf("const:     %s\n", \implode(" ", $folded));
}
