<?php
$q = sodium_crypto_generichash('msg');
var_dump(bin2hex($q));
$q = sodium_crypto_generichash('msg', '0123456789abcdef');
var_dump(bin2hex($q));
$q = sodium_crypto_generichash('msg', '0123456789abcdef', 64);
var_dump(bin2hex($q));
$q = sodium_crypto_generichash('msg', '0123456789abcdef0123456789abcdef', 64);
var_dump(bin2hex($q));
$state = sodium_crypto_generichash_init();
$q = sodium_crypto_generichash_final($state);
var_dump(bin2hex($q));
$state = sodium_crypto_generichash_init();
sodium_crypto_generichash_update($state, 'm');
sodium_crypto_generichash_update($state, 'sg');
$q = sodium_crypto_generichash_final($state);
var_dump(bin2hex($q));
$state = sodium_crypto_generichash_init('0123456789abcdef');
sodium_crypto_generichash_update($state, 'm');
sodium_crypto_generichash_update($state, 'sg');
$q = sodium_crypto_generichash_final($state);
var_dump(bin2hex($q));
$state = sodium_crypto_generichash_init('0123456789abcdef', 64);
sodium_crypto_generichash_update($state, 'm');
sodium_crypto_generichash_update($state, 'sg');
$state2 = '' . $state;
$q = sodium_crypto_generichash_final($state, 64);
var_dump(bin2hex($q));
sodium_crypto_generichash_update($state2, '2');
$q = sodium_crypto_generichash_final($state2, 64);
$exp = bin2hex($q);
var_dump($exp);
$act = bin2hex(
	sodium_crypto_generichash('msg2', '0123456789abcdef', 64)
);
var_dump($act);
var_dump($exp === $act);
try {
	$hash = sodium_crypto_generichash('test', '', 128);
} catch (SodiumException $ex) {
	var_dump(true);
}
?>
