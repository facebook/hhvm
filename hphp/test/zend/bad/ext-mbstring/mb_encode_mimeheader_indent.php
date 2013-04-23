<?php
/* Prototype  : string mb_encode_mimeheader
 * (string $str [, string $charset [, string $transfer_encoding [, string $linefeed [, int $indent]]]])
 * Description: Converts the string to MIME "encoded-word" in the format of =?charset?(B|Q)?encoded_string?= 
 * Source code: ext/mbstring/mbstring.c
 */

/*
 * Pass different data types to $indent argument to see how mb_encode_mimeheader() behaves
 */

echo "*** Testing mb_encode_mimeheader() : indent ***\n";

mb_internal_encoding('utf-8');

// Initialise function arguments not being substituted
$str = base64_decode('zpHPhc+Ez4wgzrXOr869zrHOuSDOtc67zrvOt869zrnOus+MIM66zrXOr868zrXOvc6/LiAwMTIzNDU2Nzg5Lg==');
$charset = 'utf-8';
$linefeed = "\r\n";

for ($i = 0; $i < 100; $i++) {
  echo "\n-- Iteration $i --\n";
  var_dump( mb_encode_mimeheader($str, $charset, "B", $linefeed, $i));
  var_dump( mb_encode_mimeheader($str, $charset, "Q", $linefeed, $i));  
};
echo "Done";
?>