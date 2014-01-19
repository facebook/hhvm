<?php

$key = <<<EOF
-----BEGIN PRIVATE KEY-----
MIICdQIBADANBgkqhkiG9w0BAQEFAASCAl8wggJbAgEAAoGBALxTCsLc9GVvv9Hj
riy0tyAS4LCuQvmca0NkJYAyWO3VoJ68oic+6qqCCeJZUw0itpcuzuaF73/8AmDN
ULjXvvHIkrjdkx0E24AUyjbqtdFyBH2BsidLyEbhBSFFwVnxtkGR1WS+BLD2UQCN
jeMhaqfnvJ7Blb+X8UFd1qYM4BXLAgMBAAECgYAIUC4gbfqH48f27vMIXC3dR5gN
lJO2SxZdEjWplA9i6FQ/zZnm25smTbk7+a912/ttbw6JFI5++tPsDAQtNLDkDf1r
SIYzpj/wvbwEMj8Ezod8agRG1/Gayo/YKtKfE+rnlwnyHWPf5au7UARoXNz+9Zjg
BiDsYaQpa0gWJOh+6QJBAO/vEvgHg2+NKeBK9gupMrjCakh/V+473jEpWP4LSoDd
TX3KhW0eYvm/oNPbzXSmI7vRZJIKYwDPoE1DbIy3gA0CQQDI70hHox9thaS5V1nN
VRaUEQ6T1ssZMehczJYMLS0JfkuZb+nnr8ULjjCVLJvjsBdG7Dv0hs236bLP2DVu
Oh83AkAQGP0v0Ok7mb/+gWkCnUZ+6ORSmuCeZjvhFoIXAVDtVmu1jdnn2UsQsI+s
xymAswjguArEVZgQ3N1HccedpU4dAkBuuXzpDPi6j5SQFZSE08iXWzbfPNO6VIgo
6wwcNaDFxHTAXq1UYMWCcp5O9cAJnfWVvYPkYYxnNj60zf3Tee/NAkBhjaquRQ8b
Ol3NL1UvZIzrhb8pFemgqPktS+AYgmYaOiEJEJp2NcGN1JCCR7zPsfDHo/heSY+w
bHgjQ5gI077k
-----END PRIVATE KEY-----
EOF;

$res = openssl_pkey_get_private($key);
var_dump(openssl_pkey_get_details($res));
?>
