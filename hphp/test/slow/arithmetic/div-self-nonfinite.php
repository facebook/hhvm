<?hh

/*
 * Dividing a double by itself is not 1.0 when the double is not finite:
 * INF / INF, -INF / -INF and NAN / NAN are all NaN.  The identity does hold
 * for integers, so it is tempting to write it for doubles too.
 *
 * Each quotient is checked against the same division with the two operands
 * held in separate variables.  A simplifier can recognise `$x / $x` as one
 * value divided by itself, and cannot do that when the operands arrive
 * separately, so the two forms disagreeing means the folded one is wrong.
 *
 * Results are reported by class rather than by bit pattern: which NaN a
 * division produces differs between architectures, but whether it is a NaN
 * does not.
 */

function opaque(float $v): float {
  __hhvm_intrinsics\launder_value_inout(inout $v);
  return $v;
}

// Both operands are the same value, so this is the form that can be folded.
function divSelf(float $x): float { return $x / $x; }

// Same arithmetic, two separate operands.
function divPair(float $x, float $y): float { return $x / $y; }

function describe(float $f): string {
  if (\is_nan($f)) return "NAN";
  if (\is_infinite($f)) return $f > 0.0 ? "INF" : "-INF";
  return (string)$f;
}

<<__EntryPoint>>
function main(): void {
  $inputs = vec[
    tuple("-INF", -\INF),
    tuple("INF", \INF),
    tuple("NAN", \NAN),
    tuple("2.5", 2.5),
    tuple("-1.0", -1.0),
    tuple("1.0", 1.0),
  ];

  // Enough calls to get both forms translated, and retranslated to Optimize
  // under --retranslate-all.
  foreach ($inputs as $in) {
    $self = 0.0;
    $pair = 0.0;
    for ($i = 0; $i < 2000; ++$i) {
      $x = opaque($in[1]);
      $self = divSelf($x);
      $pair = divPair($x, opaque($in[1]));
    }
    printf("%s / itself = %s", $in[0], describe($self));
    if (describe($self) !== describe($pair)) {
      printf("   MISMATCH: separate operands give %s", describe($pair));
    }
    printf("\n");
  }
}
