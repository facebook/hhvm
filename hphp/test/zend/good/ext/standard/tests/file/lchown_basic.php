<?hh
/* Prototype  : bool lchown (string filename, mixed user)
 * Description: Change file owner of a symlink
 * Source code: ext/standard/filestat.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing lchown() : basic functionality ***\n";
$filename = sys_get_temp_dir().'/'.'lchown_basic.txt';
$symlink = sys_get_temp_dir().'/'.'lchown_basic_symlink.txt';

$uid = posix_getuid();

var_dump( touch( $filename ) );
var_dump( symlink( $filename, $symlink ) );
var_dump( lchown( $filename, $uid ) );
var_dump( fileowner( $symlink ) === $uid );

echo "===DONE===\n";

unlink($filename);
unlink($symlink);
}
