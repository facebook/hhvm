<?php
/* Prototype: array glob ( string $pattern [, int $flags] );
   Description: Find pathnames matching a pattern
*/

echo "*** Testing glob() : usage variations ***\n";

$file_path = dirname(__FILE__);

// temp dir created
mkdir("$file_path/glob_variation");
mkdir("$file_path/glob_variation/wonder");

// temp files created
$fp = fopen("$file_path/glob_variation/wonder12345", "w");
fclose($fp);
$fp = fopen("$file_path/glob_variation/wonder;123456", "w");
fclose($fp);

$patterns = array (
  "$file_path/glob_variation/*der*",
  "$file_path/glob_variation/?onder*",
  "$file_path/glob_variation/w*der?*",
  "$file_path/glob_variation/*der5",
  "$file_path/glob_variation/??onder*",
  "$file_path/glob_variation/***der***",
  "$file_path/glob_variation/++onder*",
  "$file_path/glob_variation/WONDER5\0",
  '$file_path/glob_variation/wonder5',
  "$file_path/glob_variation/?wonder?",
  "$file_path/glob_variation/wonder?",
  TRUE  // boolean true
);
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
var_dump( glob("$file_path/glob_variation/*{5}", GLOB_BRACE) );

// delete temp files and dir
unlink("$file_path/glob_variation/wonder12345");
unlink("$file_path/glob_variation/wonder;123456");
rmdir("$file_path/glob_variation/wonder");
rmdir("$file_path/glob_variation");

echo "\n*** Testing glob() on directories ***\n";
// temp dir created to check for pattern matching the sub dir created in it
mkdir("$file_path/glob_variation/wonder1/wonder2", 0777, true);

$counter = 1;
/* loop through $patterns to match each $pattern with the directories created
   using glob() */
foreach($patterns as $pattern) {
  echo "-- Iteration $counter --\n";
  var_dump( glob($pattern, GLOB_ONLYDIR) );
  $counter++;
}

echo "Done\n";
?>
<?php
$file_path = dirname(__FILE__);
rmdir("$file_path/glob_variation/wonder1/wonder2");
rmdir("$file_path/glob_variation/wonder1/");
rmdir("$file_path/glob_variation/");
?>