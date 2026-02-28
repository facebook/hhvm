<?hh
/* Prototype: array glob ( string $pattern [, int $flags] );
   Description: Find pathnames matching a pattern
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing glob() : usage variations ***\n";

$dirname = sys_get_temp_dir().'/'.'glob_variation';

// temp dir created
mkdir($dirname);
mkdir("$dirname/wonder");

// temp files created
$fp = fopen("$dirname/wonder12345", "w");
fclose($fp);
$fp = fopen("$dirname/wonder;123456", "w");
fclose($fp);

$patterns = vec[
  "$dirname/*der*",
  "$dirname/?onder*",
  "$dirname/w*der?*",
  "$dirname/*der5",
  "$dirname/??onder*",
  "$dirname/***der***",
  "$dirname/++onder*",
  "$dirname/WONDER5\0",
  '$dirname/wonder5',
  "$dirname/?wonder?",
  "$dirname/wonder?",
  '1'  // boolean true
];
$counter = 1;
/* loop through $patterns to match each $pattern with the files created
   using glob() */
foreach($patterns as $pattern) {
  echo "\n-- Iteration $counter --\n";
  var_dump( glob($pattern) );  // default arguments
  var_dump( glob($pattern, GLOB_MARK) );
  var_dump( glob($pattern, GLOB_NOSORT) );
  var_dump( glob($pattern, GLOB_NOCHECK) );
  var_dump( glob($pattern, GLOB_NOESCAPE) );
  var_dump( glob($pattern, GLOB_ERR) );
  $counter++;
}

echo "\n*** Testing glob() with pattern within braces ***\n";
var_dump( glob("$dirname/*{5}", GLOB_BRACE) );

// delete temp files and dir
unlink("$dirname/wonder12345");
unlink("$dirname/wonder;123456");
rmdir("$dirname/wonder");
rmdir($dirname);

echo "\n*** Testing glob() on directories ***\n";
// temp dir created to check for pattern matching the sub dir created in it
mkdir("$dirname/wonder1/wonder2", 0777, true);

$counter = 1;
/* loop through $patterns to match each $pattern with the directories created
   using glob() */
foreach($patterns as $pattern) {
  echo "-- Iteration $counter --\n";
  var_dump( glob($pattern, GLOB_ONLYDIR) );
  $counter++;
}

echo "Done\n";

rmdir("$dirname/wonder1/wonder2");
rmdir("$dirname/wonder1");
rmdir($dirname);
}
