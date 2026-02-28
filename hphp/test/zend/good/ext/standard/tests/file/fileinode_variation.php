<?hh
/*
 * Prototype: int fileinode ( string $filename );
 * Description: Returns the inode number of the file, or FALSE in case of an error.
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing fileinode() with files, links and directories ***\n";
$file1 = sys_get_temp_dir().'/'.'fileinode1_variation.tmp';
$file2 = sys_get_temp_dir().'/'.'fileinode2_variation.tmp';
$link1 = sys_get_temp_dir().'/'.'fileinode1_variation_link.tmp';
$link2 = sys_get_temp_dir().'/'.'fileinode2_variation_link.tmp';


echo "-- Testing with files --\n";
//creating the files
fclose( fopen( $file1, "w" ) );
fclose( fopen( $file2, "w" ) );

print( fileinode( $file1) )."\n";
print( fileinode( $file2) )."\n";
clearstatcache();

echo "-- Testing with links: hard link --\n";
link( $file1, $link1);  // Creating an hard link
print( fileinode( $file1) )."\n";
clearstatcache();
print( fileinode( $link1) )."\n";
clearstatcache();

echo "-- Testing with links: soft link --\n";
symlink( $file2, $link2);  // Creating a soft link
print( fileinode( $file2) )."\n";
clearstatcache();
print( fileinode( $link2) )."\n";

unlink( $link1 );
unlink( $link2 );

echo "-- Testing after copying a file --\n";
copy( $file1, sys_get_temp_dir().'/'.'fileinode1_variation_new.tmp');
print( fileinode( $file1) )."\n";
clearstatcache();
print( fileinode( sys_get_temp_dir().'/'.'fileinode1_variation_new.tmp') ."\n");

unlink( sys_get_temp_dir().'/'.'fileinode1_variation_new.tmp');
unlink( $file1);
unlink( $file2);


echo "-- Testing after renaming the file --\n";
fclose( fopen(sys_get_temp_dir().'/'.'old.txt', "w")) ;
print( fileinode(sys_get_temp_dir().'/'.'old.txt') ."\n");
clearstatcache();

rename(
  sys_get_temp_dir().'/'.'old.txt',
  sys_get_temp_dir().'/'.'new.txt'
);
print( fileinode(sys_get_temp_dir().'/'.'new.txt') ."\n");

unlink(sys_get_temp_dir().'/'.'new.txt');

echo "-- Testing with directories --\n";
mkdir(sys_get_temp_dir().'/'.'dir');
print( fileinode(sys_get_temp_dir().'/'.'dir') ."\n");
clearstatcache();

mkdir(sys_get_temp_dir().'/'.'dir/subdir');
print( fileinode(sys_get_temp_dir().'/'.'dir/subdir') ."\n");
clearstatcache();

echo "-- Testing with binary input --\n";
print( fileinode(sys_get_temp_dir().'/'.'dir') ."\n");
clearstatcache();
print( fileinode(sys_get_temp_dir().'/'.'dir/subdir')) ;

rmdir(sys_get_temp_dir().'/'.'dir/subdir');
rmdir(sys_get_temp_dir().'/'.'dir');

echo "\n*** Done ***";
}
