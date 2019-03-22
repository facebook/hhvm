<?php
$data = "Testing openssl_sign()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$wrong = "wrong";

var_dump(openssl_sign($data, &$sign, $privkey));                 // no output
var_dump(openssl_sign($data, &$sign, $wrong));
try { var_dump(openssl_sign(array(), &$sign, $privkey)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
