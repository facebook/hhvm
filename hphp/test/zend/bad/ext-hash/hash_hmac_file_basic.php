<?php


/* Prototype  : string hash_hmac_file ( string algo, string filename, string key [, bool raw_output] )
 * Description: Generate a keyed hash value using the HMAC method and the contents of a given file
 * Source code: ext/hash/hash.c
 * Alias to functions: 
*/

echo "*** Testing hash_hmac_file() : basic functionality ***\n";

$file = dirname(__FILE__) . "hash_hmac_file.txt";
/* Creating a temporary file file */
if (($fp = fopen( $file, "w+")) == FALSE) {
	echo "Cannot create file ($file)";
    exit;
}	

/* Writing into file */ 
$content = "This is a sample string used to test the hash_hmac_file function with various hashing algorithms";
if (is_writable($file)) {
  if (fwrite($fp, $content) === FALSE) {
    echo "Cannot write to file ($file)";
    exit;
  }
}

// close the files 
fclose($fp);

$key = 'secret';


echo "adler32: " . hash_hmac_file('adler32', $file, $key) . "\n";
echo "crc32: " . hash_hmac_file('crc32', $file, $key) . "\n";
echo "gost: " . hash_hmac_file('gost', $file, $key) . "\n";
echo "haval128,3: " . hash_hmac_file('haval128,3', $file, $key) . "\n";
echo "md2: " . hash_hmac_file('md2', $file, $key) . "\n";
echo "md4: " . hash_hmac_file('md4', $file, $key) . "\n";
echo "md5: " . hash_hmac_file('md5', $file, $key) . "\n";
echo "ripemd128: " . hash_hmac_file('ripemd128', $file, $key) . "\n";
echo "ripemd160: " . hash_hmac_file('ripemd160', $file, $key) . "\n";
echo "ripemd256: " . hash_hmac_file('ripemd256', $file, $key) . "\n";
echo "ripemd320: " . hash_hmac_file('ripemd320', $file, $key) . "\n";
echo "sha1: " . hash_hmac_file('sha1', $file, $key) . "\n";
echo "sha256: " . hash_hmac_file('sha256', $file, $key) . "\n";
echo "sha384: " . hash_hmac_file('sha384', $file, $key) . "\n";
echo "sha512: " . hash_hmac_file('sha512', $file, $key) . "\n";
echo "snefru: " . hash_hmac_file('snefru', $file, $key) . "\n";
echo "tiger192,3: " . hash_hmac_file('tiger192,3', $file, $key) . "\n";
echo "whirlpool: " . hash_hmac_file('whirlpool', $file, $key) . "\n";

echo "adler32(raw): " . bin2hex(hash_hmac_file('adler32', $file, $key, TRUE)) . "\n";
echo "md5(raw): " . bin2hex(hash_hmac_file('md5', $file, $key, TRUE)). "\n";
echo "sha256(raw): " . bin2hex(hash_hmac_file('sha256', $file, $key, TRUE)). "\n";

echo "Error cases:\n";
hash_hmac_file();
hash_hmac_file('foo', $file);
hash_hmac_file('foo', $file, $key, TRUE, 10);

unlink($file);

?>
===Done===