<?php

$key = <<<EOF
-----BEGIN PUBLIC KEY-----
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQDXX/MsKEBLcLeKA1d/i7ufG1qs
qS97xFkIRSeX3TwmHic843AfVrzoh2pZUeOvK9ZLZQpHSM7DoHMYDGD1273+FvZX
Ypf5LiFtecfxko/Cku16zy6WAeCYVFjjlveBhwPmPCIk+qDRYeiIW05QE2XK+CuD
nJ7sxxXIJSSgD3Jo5wIDAQAB
-----END PUBLIC KEY-----
EOF;

print $key;
$res = openssl_pkey_get_public($key);
var_dump(openssl_pkey_get_details($res));
?>

