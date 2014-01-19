<?php
$cert = file_get_contents(__DIR__.'/bug64802.pem');
$r = openssl_x509_parse($cert,$use_short_names=true);
sort($r['subject']);
var_dump( $r['subject'] );
?>