<?php
$data = "Testing openssl_public_decrypt()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$pubkey = "file://" . dirname(__FILE__) . "/public.key";
$wrong = "wrong";

openssl_public_encrypt($data, $encrypted, $pubkey);
var_dump(openssl_private_decrypt($encrypted, $output, $privkey));
var_dump($output);
var_dump(openssl_private_decrypt($encrypted, $output2, $wrong));
var_dump($output2);
var_dump(openssl_private_decrypt($wrong, $output3, $privkey));
var_dump($output3);
var_dump(openssl_private_decrypt($encrypted, $output4, array($privkey)));
var_dump($output4);
var_dump(openssl_private_decrypt($encrypted, $output5, array($privkey, "")));
var_dump($output5);
?>