<?php

$cert = "file://" . dirname(__FILE__) . "/cert.crt";

echo "** Testing with no parameters **\n";
var_dump(openssl_x509_fingerprint());

echo "** Testing default functionality **\n";
var_dump(openssl_x509_fingerprint($cert));

echo "** Testing hash method md5 **\n";
var_dump(openssl_x509_fingerprint($cert, 'md5'));

echo "**Testing raw output md5 **\n";
var_dump(bin2hex(openssl_x509_fingerprint($cert, 'md5', true)));

echo "** Testing bad certification **\n";
var_dump(openssl_x509_fingerprint('123'));
echo "** Testing bad hash method **\n";
var_dump(openssl_x509_fingerprint($cert, 'xx45'));
