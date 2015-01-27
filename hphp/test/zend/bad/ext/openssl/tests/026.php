<?php
$x = openssl_pkey_new();
$csr = openssl_csr_new(["countryName" => "DE"], $x, ["x509_extensions" => 0xDEADBEEF]);
?>
DONE
