<?hh

/*
   +-------------------------------------------------------------+
   | Copyright (c) 2015 Facebook, Inc. (http://www.facebook.com) |
   +-------------------------------------------------------------+
*/
<<__EntryPoint>> function main(): void {
error_reporting(-1);

// check for even integer values by inspecting the low-order bit

for ($i = -5; $i <= 5; ++$i)
    echo "$i is ".(($i & (int)(HH\Lib\Legacy_FIXME\eq(1, TRUE))) ? "odd\n" : "even\n");

$upCaseLetter = 0x41;                   // letter 'A'
$lowCaseLetter = $upCaseLetter | 0x20;  // set the 6th bit
printf("Lowercase equivalent of '%c' is '%c'\n", $upCaseLetter, $lowCaseLetter);

$lowCaseLetter = 0x73;                  // letter 's'
$upCaseLetter = $lowCaseLetter & ~0x20; // clear the 6th bit
printf("Uppercase equivalent of '%c' is '%c'\n", $lowCaseLetter, $upCaseLetter);

// swap two integers

$v1 = 1234; $v2 = -987;
printf("\$v1 = $v1, \$v2 = $v2\n", $v1, $v2);
$v1 = $v1 ^ $v2;
$v2 = $v1 ^ $v2;
$v1 = $v1 ^ $v2;
printf("\$v1 = $v1, \$v2 = $v2\n", $v1, $v2);

printf("0b101101 & 0b111 = 0b%b\n", 0b101111 & 0b101);
printf("0b101101 | 0b111 = 0b%b\n", 0b101111 | 0b101);
printf("0b101101 ^ 0b111 = 0b%b\n", 0b101111 ^ 0b101);

// Test all kinds of scalar values to see which are ints or can be implicitly converted

$scalarValueList = vec[10, -100];//, 0, 1.234, 0.0, TRUE, FALSE, NULL, "123", 'xx', "");
foreach ($scalarValueList as $v)
{
    printf("%b & 123 = %b\n", $v, $v & 123);
    printf("%b | 123 = %b\n", $v, $v | 123);
    printf("%b ^ 123 = %b\n", $v, $v ^ 123);
}
}
