<?hh
/*
* proto string hash_hmac ( string algo, string data, string key [, bool raw_output] )
* Function is implemented in ext/hash/hash.c
*/
<<__EntryPoint>> function main(): void {
echo "*** Testing hash_hmac() : error conditions ***\n";

$data = "This is a sample string used to test the hash_hmac function with various hashing algorithms";
$key = 'secret';

echo "\n-- Testing hash_hmac() function with less than expected no. of arguments --\n";
try { var_dump(hash_hmac()); } catch (Exception $e) { var_dump($e->getMessage()); }
try { var_dump(hash_hmac('crc32')); } catch (Exception $e) { var_dump($e->getMessage()); }
try { var_dump(hash_hmac('crc32', $data)); } catch (Exception $e) { var_dump($e->getMessage()); }

echo "\n-- Testing hash_hmac() function with more than expected no. of arguments --\n";
$extra_arg = 10;
var_dump(hash_hmac('crc32', $data, $key, false, $extra_arg));

echo "\n-- Testing hash_hmac() function with invalid hash algorithm --\n";
var_dump(hash_hmac('foo', $data, $key));
echo "===Done===";
}
