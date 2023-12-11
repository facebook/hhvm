<?hh

const MAX_64Bit = 9223372036854775807;
const MAX_32Bit = 2147483647;
const MIN_64Bit = -9223372036854775807 - 1;
const MIN_32Bit = -2147483647 - 1;
<<__EntryPoint>> function main(): void {
$validIdentical = varray [
MAX_32Bit, vec[MAX_32Bit],
MIN_32Bit, vec[MIN_32Bit],
MAX_64Bit, vec[MAX_64Bit],
MIN_64Bit, vec[MIN_64Bit],
];

$invalidIdentical = varray [
MAX_32Bit, vec["2147483647", "2147483647.0000000", 2.147483647e9, 2147483647.0, "2147483648", 2.1474836470001e9, MAX_32Bit - 1, MAX_32Bit + 1],
MIN_32Bit, vec["-2147483648", "-2147483648.000", -2.147483648e9, -2147483648.0, "-2147483649", -2.1474836480001e9, MIN_32Bit -1, MIN_32Bit + 1],
MAX_64Bit, vec[MAX_64Bit - 1, MAX_64Bit + 1],
MIN_64Bit, vec[MIN_64Bit + 1, MIN_64Bit - 1],
];


$failed = false;
// test for valid values
for ($i = 0; $i < count($validIdentical); $i +=2) {
   $typeToTestVal = $validIdentical[$i];
   $compares = $validIdentical[$i + 1];
   foreach($compares as $compareVal) {
      if ($typeToTestVal === $compareVal) {
         // do nothing
      }
      else {
         echo "FAILED: '$typeToTestVal' !== '$compareVal'\n";
         $failed = true;
      }
   }
}
// test for invalid values
for ($i = 0; $i < count($invalidIdentical); $i +=2) {
   $typeToTestVal = $invalidIdentical[$i];
   $compares = $invalidIdentical[$i + 1];
   foreach($compares as $compareVal) {
      if ($typeToTestVal === $compareVal) {
         echo "FAILED: '$typeToTestVal' === '$compareVal'\n";
         $failed = true;
      }
   }
}

if ($failed == false) {
   echo "Test Passed\n";
}

echo "===DONE===\n";
}
