<?php
$cert = "file://" . dirname(__FILE__) . "/cert.crt";

var_dump(openssl_x509_parse($cert));
var_dump(openssl_x509_parse($cert, false));
?>