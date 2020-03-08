<?hh // strict

namespace NS_bitwise_shift;

function main(): void {
  $i32 = 1 << 31;	// if this is negative, we have a 32-bit int
  $NumBitsPerInt = ($i32 < 0) ? 32 : 64;

// Shift a positive value right and left using both in- and out-of-range counts

  $v = 1000;
  for ($i = -$NumBitsPerInt - 1; $i <= $NumBitsPerInt + 1; ++$i) {
    printf("%d(%08X): >> %2d = %08X\t<< %2d = %08X\n", $v, $v, $i, $v >> $i, $i, $v << $i);
  }

// Shift a negative value right and left using both in- and out-of-range counts

  $v = -1000;
  for ($i = -$NumBitsPerInt - 1; $i <= $NumBitsPerInt + 1; ++$i) {
    printf("%d(%08X): >> %2d = %08X\t<< %2d = %08X\n", $v, $v, $i, $v >> $i, $i, $v << $i);
  }

// Figure out the algorithm the implementations use for negative and too-large shift counts

  for ($i = -129; $i <= 129; ++$i) {
    $rem = $i % $NumBitsPerInt;
    if ($rem == 0 || $i > 0) {
      echo "$i, ".$rem."\n";
    } else {  	// have a negative shift
      $r = $NumBitsPerInt - (-$i % $NumBitsPerInt);
      echo "$i, ".$r."\n";
    }
  }

// Shift all kinds of scalar values

  $v = 10 << 2;
//  $v = 1.234 << 2;	// non-numeric operand not allowed
//  $v = true << 2;	// ...
//  $v = null << 2;	// ...
//  $v = "123" << 2;	// ...
//  $v = 10 << 2.0;	// non-numeric operand not allowed

  $v = 10 >> 2;
//  $v = 1.234 >> 2;	// non-numeric operand not allowed
//  $v = true >> 2;	// ...
//  $v = null >> 2;	// ...
//  $v = "123" >> 2;	// ...
}

/* HH_FIXME[1002] call to main in strict*/
main();
