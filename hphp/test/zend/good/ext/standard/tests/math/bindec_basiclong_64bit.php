<?hh

const MAX_64Bit = 9223372036854775807;
const MAX_32Bit = 2147483647;
const MIN_64Bit = -9223372036854775807 - 1;
const MIN_32Bit = -2147483647 - 1;
<<__EntryPoint>> function main(): void {
$binLongStrs = vec[
   '0'.str_repeat('1',63),
   str_repeat('1',64),
   '0'.str_repeat('1',31),
   str_repeat('1',32),
   '0'.str_repeat('1',64),
   str_repeat('1',65),
   '0'.str_repeat('1',32),
   str_repeat('1',33)
];


foreach ($binLongStrs as $strVal) {
   echo "--- testing: $strVal ---\n";
   var_dump(bindec($strVal));
}

echo "===DONE===\n";
}
