<?php
$cert = file_get_contents(__DIR__ . '/cve2013_4073.pem');
$info = openssl_x509_parse($cert);
var_export($info['extensions']);
