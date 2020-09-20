<?hh
/* Prototype : bool unlink ( string $filename [, resource $context] );
   Description : Deletes filename
*/

/* Delete link files - soft and hard links */
<<__EntryPoint>> function main(): void {
// temp file used
$filename = __SystemLib\hphp_test_tmppath('unlink_variation3.tmp');

echo "*** Testing unlink() on soft and hard links ***\n";
// create temp file
$fp = fopen($filename, "w");
fclose($fp);
// link name used here
$linkname = __SystemLib\hphp_test_tmppath('unlink_variation3_link.tmp');

echo "-- Testing unlink() on soft link --\n";
// create soft link
var_dump( symlink($filename, $linkname) );  // expected: true
// unlink soft link
var_dump( unlink($linkname) );  // expected: true
var_dump( file_exists($linkname) );  // confirm link is deleted

echo "-- Testing unlink() on hard link --\n";
// create hard link
var_dump( link($filename, $linkname) );  // expected: true
// delete hard link
var_dump( unlink($linkname) );  // expected: true
var_dump( file_exists($linkname) );  // confirm link is deleted

// delete temp file
var_dump( unlink($filename) );
var_dump( file_exists($filename) );  // confirm file is deleted

echo "Done\n";
}
