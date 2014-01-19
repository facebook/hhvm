<?php
$infile = dirname(__FILE__) . "/cert.crt";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$encrypted = tempnam("/tmp", "ssl");
if ($encrypted === false)
	die("failed to get a temporary filename!");
$outfile = tempnam("/tmp", "ssl");
if ($outfile === false) {
	unlink($outfile);
	die("failed to get a temporary filename!");
}

$single_cert = "file://" . dirname(__FILE__) . "/cert.crt";
$headers = array("test@test", "testing openssl_pkcs7_encrypt()");
$wrong = "wrong";
$empty = "";

openssl_pkcs7_encrypt($infile, $encrypted, $single_cert, $headers);
var_dump(openssl_pkcs7_decrypt($encrypted, $outfile, $single_cert, $privkey));
var_dump(openssl_pkcs7_decrypt($encrypted, $outfile, $single_cert, $wrong));
var_dump(openssl_pkcs7_decrypt($encrypted, $outfile, $wrong, $privkey));
var_dump(openssl_pkcs7_decrypt($encrypted, $outfile, null, $privkey));
var_dump(openssl_pkcs7_decrypt($wrong, $outfile, $single_cert, $privkey));
var_dump(openssl_pkcs7_decrypt($empty, $outfile, $single_cert, $privkey));
var_dump(openssl_pkcs7_decrypt($encrypted, $empty, $single_cert, $privkey));
var_dump(openssl_pkcs7_decrypt($encrypted, $outfile, $empty, $privkey));
var_dump(openssl_pkcs7_decrypt($encrypted, $outfile, $single_cert, $empty));

if (file_exists($encrypted)) {
	echo "true\n";
	unlink($encrypted);
}
if (file_exists($outfile)) {
	echo "true\n";
	unlink($outfile);
}
?>