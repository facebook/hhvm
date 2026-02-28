<?hh

const MAX_64Bit = 9223372036854775807;
const MAX_32Bit = 2147483647;
const MIN_64Bit = -9223372036854775807 - 1;
const MIN_32Bit = -2147483647 - 1;
<<__EntryPoint>> function main(): void {
$validGtOrEqual = vec[
MAX_32Bit, vec[MAX_32Bit, "2147483647", "2147483647.0000000", 2.147483647e9, 2147483647.0, MAX_32Bit - 1],
MIN_32Bit, vec[MIN_32Bit, "-2147483648", "-2147483648.000", -2.147483648e9, -2147483648.0, MIN_32Bit - 1],
MAX_64Bit, vec[MAX_64Bit, MAX_64Bit + 1, MAX_64Bit - 1],
MIN_64Bit, vec[MIN_64Bit, MIN_64Bit - 1],
];

$invalidGtOrEqual = vec[
MAX_32Bit, vec["2147483648", 2.1474836470001e9, MAX_32Bit + 1],
MIN_32Bit, vec[MIN_32Bit + 1,"-2147483646", -2.1474836460001e9]
];


$failed = false;
// test valid values
for ($i = 0; $i < count($validGtOrEqual); $i +=2) {
   $typeToTestVal = $validGtOrEqual[$i];
   $compares = $validGtOrEqual[$i + 1];
   foreach($compares as $compareVal) {
      if (HH\Lib\Legacy_FIXME\gte($typeToTestVal, $compareVal)) {
         // do nothing
      }
      else {
         echo "FAILED: '$typeToTestVal' < '$compareVal'\n";
         $failed = true;
      }
   }
}
// test for invalid values
for ($i = 0; $i < count($invalidGtOrEqual); $i +=2) {
   $typeToTestVal = $invalidGtOrEqual[$i];
   $compares = $invalidGtOrEqual[$i + 1];
   foreach($compares as $compareVal) {
      if (HH\Lib\Legacy_FIXME\gte($typeToTestVal, $compareVal)) {
         echo "FAILED: '$typeToTestVal' >= '$compareVal'\n";
         $failed = true;
      }
   }
}

if ($failed == false) {
   echo "Test Passed\n";
}

echo "===DONE===\n";
}
