<?hh
/*
 * Prototype: int fileinode ( string $filename );
 * Description: Returns the inode number of the file, or FALSE in case of an error.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing fileinode() with file, directory ***\n";

/* Getting inode of created file */
fopen(__SystemLib\hphp_test_tmppath('inode.tmp'), "w");
print( fileinode(__SystemLib\hphp_test_tmppath('inode.tmp')) )."\n";

/* Getting inode of current file */
print( fileinode(__FILE__) )."\n";

/* Getting inode of directories */
print( fileinode(".") )."\n";
print( fileinode("./..") )."\n";

echo "\n*** Done ***";

unlink (__SystemLib\hphp_test_tmppath('inode.tmp'));
}
