<?php
$data = "Testing openssl_public_encrypt()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$pubkey = "file://" . dirname(__FILE__) . "/public.key";
$wrong = "wrong";
class test {
        function __toString() {
                return "test";
        }
}
$obj = new test;

var_dump(openssl_public_encrypt($data, $encrypted, $pubkey));
var_dump(openssl_public_encrypt($data, $encrypted, $privkey));
var_dump(openssl_public_encrypt($data, $encrypted, $wrong));
var_dump(openssl_public_encrypt($data, $encrypted, $obj));
var_dump(openssl_public_encrypt($obj, $encrypted, $pubkey));
openssl_private_decrypt($encrypted, $output, $privkey);
var_dump($output);
?>