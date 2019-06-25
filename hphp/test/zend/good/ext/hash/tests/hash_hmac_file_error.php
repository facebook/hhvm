<?hh

/* Prototype  : string hash_hmac_file ( string algo, string filename, string key [, bool raw_output] )
 * Description: Generate a keyed hash value using the HMAC method and the contents of a given file
 * Source code: ext/hash/hash.c
 * Alias to functions:
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing hash() : error conditions ***\n";

$file = dirname(__FILE__) . "hash_file.txt";
$key = 'secret';

echo "\n-- Testing hash_hmac_file() function with less than expected no. of arguments --\n";
try { var_dump(hash_hmac_file()); } catch (Exception $e) { var_dump($e->getMessage()); }
try { var_dump(hash_hmac_file('crc32')); } catch (Exception $e) { var_dump($e->getMessage()); }
try { var_dump(hash_hmac_file('crc32', $file)); } catch (Exception $e) { var_dump($e->getMessage()); }

echo "\n-- Testing hash_hmac_file() function with more than expected no. of arguments --\n";
$extra_arg = 10;
hash_hmac_file('crc32', $file, $key, TRUE, $extra_arg);

echo "\n-- Testing hash_hmac_file() function with invalid hash algorithm --\n";
hash_hmac_file('foo', $file, $key, TRUE);
echo "===Done===";
}
