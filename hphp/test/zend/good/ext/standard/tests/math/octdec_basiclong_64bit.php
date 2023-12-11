<?hh

const MAX_64Bit = 9223372036854775807;
const MAX_32Bit = 2147483647;
const MIN_64Bit = -9223372036854775807 - 1;
const MIN_32Bit = -2147483647 - 1;
<<__EntryPoint>> function main(): void {
$octLongStrs = vec[
   '777777777777777777777',
   '1777777777777777777777',
   '17777777777',
   '37777777777',
   '377777777777777777777777',
   '17777777777777777777777777',
   '377777777777',
   '777777777777',
];


foreach ($octLongStrs as $strVal) {
   echo "--- testing: $strVal ---\n";
   var_dump(octdec($strVal));
}

echo "===DONE===\n";
}
