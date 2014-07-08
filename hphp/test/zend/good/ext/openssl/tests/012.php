<?php
$data = "openssl_open() test";
$pub_key = "file://" . dirname(__FILE__) . "/public.key";
$wrong = "wrong";

openssl_seal($data, $sealed, $ekeys, array($pub_key));                  // no output
openssl_seal($data, $sealed, $ekeys, array($pub_key, $pub_key));        // no output
openssl_seal($data, $sealed, $ekeys, array($pub_key, $wrong));
openssl_seal($data, $sealed, $ekeys, $pub_key);
openssl_seal($data, $sealed, $ekeys, array());
openssl_seal($data, $sealed, $ekeys, array($wrong));
?>