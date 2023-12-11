<?hh

const MAX_64Bit = 9223372036854775807;
const MAX_32Bit = 2147483647;
const MIN_64Bit = -9223372036854775807 - 1;
const MIN_32Bit = -2147483647 - 1;
<<__EntryPoint>> function main(): void {
$longVals = vec[
    MAX_64Bit, MIN_64Bit, MAX_32Bit, MIN_32Bit, MAX_64Bit - MAX_32Bit, MIN_64Bit - MIN_32Bit,
    MAX_32Bit + 1, MIN_32Bit - 1, MAX_32Bit * 2, (MAX_32Bit * 2) + 1, (MAX_32Bit * 2) - 1,
    MAX_64Bit -1, MAX_64Bit + 1, MIN_64Bit + 1, MIN_64Bit - 1
];

$otherVals = vec[0, 1, -1, 7, 9, 65, -44, MAX_32Bit, MAX_64Bit];

error_reporting(E_ERROR);

foreach ($longVals as $longVal) {
  foreach($otherVals as $otherVal) {
    echo "--- testing: $longVal / $otherVal ---\n";
    try {
      var_dump($longVal/$otherVal);
    } catch (DivisionByZeroException $e) {
      echo $e->getMessage(), "\n";
    }
  }
}

foreach ($otherVals as $otherVal) {
   foreach($longVals as $longVal) {
       echo "--- testing: $otherVal / $longVal ---\n";
      var_dump($otherVal/$longVal);
   }
}

echo "===DONE===\n";
}
