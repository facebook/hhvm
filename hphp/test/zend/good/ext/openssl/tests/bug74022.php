<?php
$pfx = dirname(__FILE__) . DIRECTORY_SEPARATOR . "bug74022.pfx";
$cert_store = file_get_contents($pfx);

var_dump(openssl_pkcs12_read($cert_store, &$cert_info, "csos"));
var_dump(openssl_error_string());
