<?php
$pkey = 'ASN1 OID: prime256v1
-----BEGIN EC PARAMETERS-----
BggqhkjOPQMBBw==
-----END EC PARAMETERS-----
-----BEGIN EC PRIVATE KEY-----
MHcCAQEEILPkqoeyM7XgwYkuSj3077lrsrfWJK5LqMolv+m2oOjZoAoGCCqGSM49
AwEHoUQDQgAEPq4hbIWHvB51rdWr8ejrjWo4qVNWVugYFtPg/xLQw0mHkIPZ4DvK
sqOTOnMoezkbSmVVMuwz9flvnqHGmQvmug==
-----END EC PRIVATE KEY-----';
$key = openssl_pkey_get_private($pkey);
$res = openssl_sign($data ='alpha', $sign, $key, 'ecdsa-with-SHA1');
var_dump($res);
