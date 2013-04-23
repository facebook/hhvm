<?php 
$priv = openssl_pkey_new();
$pub = openssl_pkey_get_public($priv);
?>