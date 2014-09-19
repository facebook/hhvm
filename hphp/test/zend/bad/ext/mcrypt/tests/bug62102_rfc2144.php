<?php
$plaintext = "\x01\x23\x45\x67\x89\xAB\xCD\xEF";

echo

"128-bit: ",
bin2hex(mcrypt_encrypt('cast-128', "\x01\x23\x45\x67\x12\x34\x56\x78\x23\x45\x67\x89\x34\x56\x78\x9A", $plaintext, 'ecb')),
"\n",
"80-bit: ",
bin2hex(mcrypt_encrypt('cast-128', "\x01\x23\x45\x67\x12\x34\x56\x78\x23\x45", $plaintext, 'ecb')),
"\n",
"40-bit: ",
bin2hex(mcrypt_encrypt('cast-128', "\x01\x23\x45\x67\x12", $plaintext, 'ecb')),
"\n";

?>
