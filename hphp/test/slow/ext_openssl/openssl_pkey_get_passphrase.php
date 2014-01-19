<?php
$_privateKeyString = <<<PK
-----BEGIN RSA PRIVATE KEY-----
Proc-Type: 4,ENCRYPTED
DEK-Info: DES-EDE3-CBC,692C329D4C517E8C

xmv90bzhJPFudKQQcMtOTirYthJC/At4u9F6IzaRMcUDS/f9MhJ6OPkxs37lsXfE
R/gRcw5fhoJNSCQLEPgNzZy11F46KAF9eVlFmBb92I5zHsGKtWIZaKkK8nthCJFU
w/FucpOF/i80PTN+QEaESeGEhr/ND5jAS10v756Bn0lrXz3QK/W7lIKzAP0f+gx/
P0V1NcMLATubmc5ryGEO/xJU9nzyEg4Ps+b4ITxM/DKOUewerWnmNVA/U1qiiYUL
DVwvjeLkgCzX9ggJnoqnM2ItWYeZ1e2IAeNa/skh1qyKxV5spat6DflY7L/+s0KQ
e1xIaYHYLOzmBqphF+gBu0Zr6C9xNeSdYeZGL8GxHhzrQzlA+zDFRySAav9uFBko
BM298Bl+3qtdm2QBzMxmrwqhY5fup9Nt2934zCqxQ1oY/dkTIaGOdEejXcUBIKnA
tpWumSqjMPiBVY/S5lQGCbLLU/4+2HpAfzqeg3IDjFgtvDysrnPPbCUMKrG1OfFi
PymDODUWbCvpy0a9KeC1b+oJL8TWaJwSx85137csvRbMqVExRB+NoKGj06cZlxgr
ewfRvQhbtUWP2gSNdFzzvrxrTEPd5Bw3J4V60uFKOqXfamfNrdPB35YazQ3CSqMU
eV8Al8BnmcT4mba3twR9EPIpcxc9x8sOVSfyksuZUx4C9o4lduAQhuP45LC7ghlr
hyWCfwOG+U1wF92pcXPX2+lfTb1i0TGGydLgFbdJbC69ZSQVC04DFjwuBIzNQupq
6E/G3hatYaKK2GImNUyf2lXowpXifjPX2tdg+Bor7fA=
-----END RSA PRIVATE KEY-----
PK;

$_publicKeyString = <<<PK
-----BEGIN PUBLIC KEY-----
MIGfMA0GCSqGSIb3DQEBAQUAA4GNADCBiQKBgQC01FK995TLzyCJdyfH1yKVJwJ+
sSxpKMLiRHfk39SPFEoePqCQ0PCiisvekx9gJNW2xpg2Ecx/8XQRq2Gq/1AymJQ0
wNPRgnCo0pi+eSjx8DJ+bWlvWVstxH93QgZPywQegS0VdGEkYD+70dl0jykRwQJ3
W/noSOyMhVl694yFbwIDAQAB
-----END PUBLIC KEY-----
PK;

$_passPhrase = 'test';
$publicKey = openssl_pkey_get_public(array($_publicKeyString, $_passPhrase));
$privateKey = openssl_pkey_get_private(array($_privateKeyString, $_passPhrase));
var_dump(openssl_public_encrypt($_passPhrase, $crypted, $publicKey));
var_dump(openssl_private_decrypt($crypted, $decrypted, $privateKey));
