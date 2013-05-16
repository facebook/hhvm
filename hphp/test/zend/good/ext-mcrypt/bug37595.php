<?php

$cipher_alg = MCRYPT_BLOWFISH;
$skey = array(0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15);
$key='';
foreach($skey as $t) {
	    $key .= chr($t);
		}
 
$sstr = array(1,2,3,4,5,6,7,8);
$iv='';
foreach($sstr as $s) {
    $iv .= chr($s);
}
 
$str = "12345678";
 
$td = mcrypt_module_open(MCRYPT_BLOWFISH,'',MCRYPT_MODE_CBC,'');
 
$data = Array(
	'12345678',
	'123456789',
	"\x001234567",
	'',
	'1234567812345678',
	'12345678123456789'
	);

foreach ($data as $val) {
	mcrypt_generic_init($td, $key, $iv);
	$enc = mcrypt_generic($td, $val);
	
	mcrypt_generic_deinit($td);
	
	mcrypt_generic_init($td, $key, $iv);
	var_dump($dec = @mdecrypt_generic($td, $enc));
}

mcrypt_module_close($td);

echo "Done\n";
?>