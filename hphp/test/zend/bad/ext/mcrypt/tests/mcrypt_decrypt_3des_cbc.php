<?php
/* Prototype  : string mcrypt_decrypt(string cipher, string key, string data, string mode, string iv)
 * Description: OFB crypt/decrypt data using key key with cipher cipher starting with iv 
 * Source code: ext/mcrypt/mcrypt.c
 * Alias to functions: 
 */

echo "*** Testing mcrypt_decrypt() : basic functionality ***\n";


// Initialise all required variables
$cipher = MCRYPT_3DES;
$mode = MCRYPT_MODE_CBC;

// tripledes uses keys with exactly 192 bits (24 bytes)
$keys = array(
   b'12345678', 
   b'12345678901234567890', 
   b'123456789012345678901234', 
   b'12345678901234567890123456'
);
$data1 = array(
   'IleMhoxiOthmHua4tFBHOw==',
   'EeF1s6C+w1IiHj1gdDn81g==',
   'EEuXpjZPueyYoG0LGQ199Q==',
   'EEuXpjZPueyYoG0LGQ199Q=='
);
// tripledes is a block cipher of 64 bits (8 bytes)
$ivs = array(
   b'1234', 
   b'12345678', 
   b'123456789'
);
$data2 = array(
   '+G7nGcWIxij3TZjpI9lJdQ==',
   '3bJiFMeyScxOLQcE6mZtLg==',
   '+G7nGcWIxij3TZjpI9lJdQ=='
);

$iv = b'12345678';
echo "\n--- testing different key lengths\n";
for ($i = 0; $i < sizeof($keys); $i++) {
   echo "\nkey length=".strlen($keys[$i])."\n";
   special_var_dump(mcrypt_decrypt($cipher, $keys[$i], base64_decode($data1[$i]), $mode, $iv));
}

$key = b'123456789012345678901234';
echo "\n--- testing different iv lengths\n";
for ($i = 0; $i < sizeof($ivs); $i++) {
   echo "\niv length=".strlen($ivs[$i])."\n";
   special_var_dump(mcrypt_decrypt($cipher, $key, base64_decode($data2[$i]), $mode, $ivs[$i]));
}

function special_var_dump($str) {
   var_dump(bin2hex($str));
}  
?>
===DONE===
