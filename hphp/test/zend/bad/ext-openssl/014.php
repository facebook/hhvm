<?php
$data = "Testing openssl_private_encrypt()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$pubkey = "file://" . dirname(__FILE__) . "/public.key";
$wrong = "wrong";
class test {
        function __toString() {
                return "test";
        }
}
$obj = new test;

var_dump(openssl_private_encrypt($data, $encrypted, $privkey));
var_dump(openssl_private_encrypt($data, $encrypted, $pubkey));
var_dump(openssl_private_encrypt($data, $encrypted, $wrong));
var_dump(openssl_private_encrypt($data, $encrypted, $obj));
var_dump(openssl_private_encrypt($obj, $encrypted, $privkey));
openssl_public_decrypt($encrypted, $output, $pubkey);
var_dump($output);
?>