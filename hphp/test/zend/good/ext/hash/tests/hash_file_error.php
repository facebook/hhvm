<?hh
/* Prototype  : string hash_file(string algo, string filename[, bool raw_output = false])
 * Description: Generate a hash of a given file
 * Source code: ext/hash/hash.c
 * Alias to functions: 
 */
<<__EntryPoint>> function main(): void {
echo "*** Testing hash_file() : error conditions ***\n";

// Set up file
$filename = sys_get_temp_dir().'/'.'hash_file_error_example.txt';
file_put_contents( $filename, 'The quick brown fox jumped over the lazy dog.' );


// hash_file() error tests
echo "\n-- Testing hash_file() function with an unknown algorithm --\n";
var_dump( hash_file( 'foobar', $filename ) );

echo "\n-- Testing hash_file() function with a non-existent file --\n";
var_dump( hash_file( 'md5', 'nonexistent.txt' ) );

echo "\n-- Testing hash_file() function with less than expected no. of arguments --\n";
try { var_dump( hash_file( 'md5' ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "\n-- Testing hash_file() function with more than expected no. of arguments --\n";
$extra_arg = 10;
try { var_dump( hash_file( 'md5', $filename, false, $extra_arg ) ); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }

echo "===DONE===\n";

unlink( $filename );
}
