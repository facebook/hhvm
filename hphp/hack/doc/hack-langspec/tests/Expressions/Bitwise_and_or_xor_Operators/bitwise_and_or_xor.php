<?hh // strict

namespace NS_bitwise_and_or_xor;

function main(): void {
///*
  echo "======= check for even/odd integer values by inspecting the low-order bit ========\n";

  for ($i = -5; $i <= 5; ++$i) {
    echo "$i is ".((($i & 1) == 1) ? "odd\n" : "even\n");
  }

  $upCaseLetter = 0x41;	// letter 'A'
  $lowCaseLetter = $upCaseLetter | 0x20;	// set the 6th bit
  printf("Lowercase equivalent of '%c' is '%c'\n", $upCaseLetter, $lowCaseLetter);

  $lowCaseLetter = 0x73;	// letter 's'
  $upCaseLetter = $lowCaseLetter & ~0x20;	// clear the 6th bit
  printf("Uppercase equivalent of '%c' is '%c'\n", $lowCaseLetter, $upCaseLetter);
//*/
///*
  echo "======= swap two integers ========\n";

  $v1 = 1234; $v2 = -987;
  printf("\$v1 = %d, \$v2 = %d\n", $v1, $v2);
  $v1 = $v1 ^ $v2;
  $v2 = $v1 ^ $v2;
  $v1 = $v1 ^ $v2;
  printf("\$v1 = %d, \$v2 = %d\n", $v1, $v2);

  echo "======= misc stuff ========\n";

  printf("0b101101 & 0b111 = 0b%b\n", 0b101111 & 0b101);
  printf("0b101101 | 0b111 = 0b%b\n", 0b101111 | 0b101);
  printf("0b101101 ^ 0b111 = 0b%b\n", 0b101111 ^ 0b101);
//*/
///*
  echo "======= Test all kinds of scalar values to see which are ints or can be implicitly converted ========\n";

// those elements comments out are rejected; only int operands are accepted

  $scalarValueList = array(-3, 0, 1000 /*, 1.234, 0.0, true, false, null, "123", 'xx', ""*/);
  foreach ($scalarValueList as $v) {
    echo "$v & 123 = "; var_dump($v & 123);
    echo "$v | 123 = "; var_dump($v | 123);
    echo "$v ^ 123 = "; var_dump($v ^ 123);
  }
//*/
}

/* HH_FIXME[1002] call to main in strict*/
main();
