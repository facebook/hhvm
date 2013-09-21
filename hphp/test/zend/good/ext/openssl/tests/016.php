<?php
$data = "Testing openssl_public_decrypt()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$pubkey = "file://" . dirname(__FILE__) . "/public.key";
$wrong = "wrong";

openssl_private_encrypt($data, $encrypted, $privkey);
var_dump(openssl_public_decrypt($encrypted, $output, $pubkey));
var_dump($output);
var_dump(openssl_public_decrypt($encrypted, $output2, $wrong));
var_dump($output2);
var_dump(openssl_public_decrypt($wrong, $output3, $pubkey));
var_dump($output3);
var_dump(openssl_public_decrypt($encrypted, $output4, array()));
var_dump($output4);
var_dump(openssl_public_decrypt($encrypted, $output5, array($pubkey)));
var_dump($output5);
var_dump(openssl_public_decrypt($encrypted, $output6, array($pubkey, "")));
var_dump($output6);
?>