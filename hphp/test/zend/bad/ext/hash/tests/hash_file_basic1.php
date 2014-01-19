<?php

/* Prototype  : string hash_file ( string algo, string filename [, bool raw_output] )
 * Description: Generate a hash value using the contents of a given file
 * Source code: ext/hash/hash.c
 * Alias to functions: 
*/

echo "*** Testing hash_file() : basic functionality ***\n";

$file = dirname(__FILE__) . "hash_file.txt";
/* Creating a temporary file file */
if (($fp = fopen( $file, "w+")) == FALSE) {
	echo "Cannot create file ($file)";
    exit;
}	

/* Writing into file */ 
$content = "This is a sample string used to test the hash_file function with various hashing algorithms";
if (is_writable($file)) {
  if (fwrite($fp, $content) === FALSE) {
    echo "Cannot write to file ($file)";
    exit;
  }
}

// close the file 
fclose($fp);

echo "adler32: " . hash_file('adler32', $file) . "\n";
echo "crc32: " . hash_file('crc32', $file) . "\n";
echo "gost: " . hash_file('gost', $file). "\n";
echo "haval128,3: " . hash_file('haval128,3', $file). "\n";
echo "md2: " . hash_file('md2', $file). "\n";
echo "md4: " . hash_file('md4', $file). "\n";
echo "md5: " . hash_file('md5', $file). "\n";
echo "ripemd128: " . hash_file('ripemd128', $file). "\n";
echo "ripemd160: " . hash_file('ripemd160', $file). "\n";
echo "ripemd256: " . hash_file('ripemd256', $file). "\n";
echo "ripemd320: " . hash_file('ripemd320', $file). "\n";
echo "sha1: " . hash_file('sha1', $file). "\n";
echo "sha256: " . hash_file('sha256', $file). "\n";
echo "sha384: " . hash_file('sha384', $file). "\n";
echo "sha512: " . hash_file('sha512', $file). "\n";
echo "snefru: " . hash_file('snefru', $file). "\n";
echo "tiger192,3: " . hash_file('tiger192,3', $file). "\n";
echo "whirlpool: " . hash_file('whirlpool', $file). "\n";

echo "adler32(raw): " . bin2hex(hash_file('adler32', $file, TRUE)) . "\n";
echo "md5(raw): " . bin2hex(hash_file('md5', $file, TRUE)). "\n";
echo "sha256(raw): " . bin2hex(hash_file('sha256', $file, TRUE)). "\n";

unlink($file);

?>
===Done===