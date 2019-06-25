<?hh
/*
 * Prototype: int fileinode ( string $filename );
 * Description: Returns the inode number of the file, or FALSE in case of an error.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing fileinode() with file, directory ***\n";

/* Getting inode of created file */
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
fopen("$file_path/inode.tmp", "w");
print( fileinode("$file_path/inode.tmp") )."\n";

/* Getting inode of current file */
print( fileinode(__FILE__) )."\n";

/* Getting inode of directories */
print( fileinode(".") )."\n";
print( fileinode("./..") )."\n";

echo "\n*** Done ***";
error_reporting(0);
$file_path = getenv('HPHP_TEST_TMPDIR') ?? dirname(__FILE__);
unlink ($file_path."/inode.tmp");
}
