<?php
$data = "openssl_open() test";
$pub_key = "file://" . dirname(__FILE__) . "/public.key";
$wrong = "wrong";

openssl_seal($data, &$sealed, &$ekeys, array($pub_key));                  // no output
openssl_seal($data, &$sealed, &$ekeys, array($pub_key, $pub_key));        // no output
openssl_seal($data, &$sealed, &$ekeys, array($pub_key, $wrong));
try { openssl_seal($data, &$sealed, &$ekeys, $pub_key); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
openssl_seal($data, &$sealed, &$ekeys, array());
openssl_seal($data, &$sealed, &$ekeys, array($wrong));
