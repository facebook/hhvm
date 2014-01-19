<?php
/* Prototype  : string mb_decode_mimeheader(string string)
 * Description: Decodes the MIME "encoded-word" in the string 
 * Source code: ext/mbstring/mbstring.c
 * Alias to functions: 
 */

echo "*** Testing mb_decode_mimeheader() : variation ***\n";
mb_internal_encoding('iso-8859-7');

//greek in UTF-8 to be converted to iso-8859-7
$encoded_word = "=?UTF-8?B?zrHOss6zzrTOtc62zrfOuM65zrrOu868zr3Ovs6/z4DPgc+Dz4TPhc+Gz4fPiM+J?=";
var_dump(bin2hex(mb_decode_mimeheader($encoded_word)));


?>
===DONE===