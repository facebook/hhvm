<?php
$crt = substr(__FILE__, 0, -4).'.crt';
$info = openssl_x509_parse("file://$crt");
var_dump($info["validFrom"], $info["validFrom_time_t"], $info["validTo"], $info["validTo_time_t"]);
?>
Done
