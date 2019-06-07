<?hh

/* as an example how to write crc32 tests
   PHP does not have uint values, you cannot
   display crc32 like a signed integer.
   Have to find some small strings to truly reproduce
   the problem, this example being not a problem
*/
<<__EntryPoint>> function main() {
echo dechex(crc32("platform independent")) . "\n";
}
