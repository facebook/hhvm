<?php
$key = openssl_pkey_get_private("file://" . dirname(__FILE__) . "/private_ec.key");
print_r(openssl_pkey_get_details($key));
