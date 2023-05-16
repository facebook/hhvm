<?hh
/*
 * Prototype: int fileinode ( string $filename );
 * Description: Returns the inode number of the file, or FALSE in case of an error.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing fileinode() with file, directory ***\n";

/* Getting inode of created file */
fopen(sys_get_temp_dir().'/'.'inode.tmp', "w");
print( fileinode(sys_get_temp_dir().'/'.'inode.tmp') ."\n");

/* Getting inode of current file */
print( fileinode(__FILE__) )."\n";

/* Getting inode of directories */
print( fileinode(".") )."\n";
print( fileinode("./..") )."\n";

echo "\n*** Done ***";

unlink (sys_get_temp_dir().'/'.'inode.tmp');
}
