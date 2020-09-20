<?hh
/* Prototype  : bool lchown (string filename, mixed user)
 * Description: Change file owner of a symlink
 * Source code: ext/standard/filestat.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing lchown() : basic functionality ***\n";
$filename = __SystemLib\hphp_test_tmppath('lchown_basic.txt');
$symlink = __SystemLib\hphp_test_tmppath('lchown_basic_symlink.txt');

$uid = posix_getuid();

var_dump( touch( $filename ) );
var_dump( symlink( $filename, $symlink ) );
var_dump( lchown( $filename, $uid ) );
var_dump( fileowner( $symlink ) === $uid );

echo "===DONE===\n";

unlink($filename);
unlink($symlink);
}
