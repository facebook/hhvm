<?php
// official test vectors
var_dump(bin2hex(openssl_pbkdf2('password', 'salt', 20, 1)));
var_dump(bin2hex(openssl_pbkdf2('password', 'salt', 20, 2)));
var_dump(bin2hex(openssl_pbkdf2('password', 'salt', 20, 4096)));

/* really slow but should be:
string(40) "eefe3d61cd4da4e4e9945b3d6ba2158c2634e984"
var_dump(bin2hex(openssl_pbkdf2('password', 'salt', 20, 16777216)));
*/

var_dump(bin2hex(openssl_pbkdf2('passwordPASSWORDpassword', 'saltSALTsaltSALTsaltSALTsaltSALTsalt', 25, 4096)));
var_dump(bin2hex(openssl_pbkdf2("pass\0word", "sa\0lt", 16, 4096)));

?>