<?php
$infile = dirname(__FILE__) . "/cert.crt";
$outfile = tempnam("/tmp", "ssl");
if ($outfile === false)
	die("failed to get a temporary filename!");
$outfile2 = tempnam("/tmp", "ssl");
if ($outfile2 === false)
	die("failed to get a temporary filename!");

$single_cert = "file://" . dirname(__FILE__) . "/cert.crt";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$multi_certs = array($single_cert, $single_cert);
$assoc_headers = array("To" => "test@test", "Subject" => "testing openssl_pkcs7_encrypt()");
$headers = array("test@test", "testing openssl_pkcs7_encrypt()");
$empty_headers = array();
$wrong = "wrong";
$empty = "";

var_dump(openssl_pkcs7_encrypt($infile, $outfile, $single_cert, $headers));
var_dump(openssl_pkcs7_decrypt($outfile, $outfile2, $single_cert, $privkey));
var_dump(openssl_pkcs7_encrypt($infile, $outfile, $single_cert, $assoc_headers));
var_dump(openssl_pkcs7_encrypt($infile, $outfile, $single_cert, $empty_headers));
var_dump(openssl_pkcs7_encrypt($infile, $outfile, $single_cert, $wrong));
var_dump(openssl_pkcs7_encrypt($wrong, $outfile, $single_cert, $headers));
var_dump(openssl_pkcs7_encrypt($empty, $outfile, $single_cert, $headers));
var_dump(openssl_pkcs7_encrypt($infile, $empty, $single_cert, $headers));
var_dump(openssl_pkcs7_encrypt($infile, $outfile, $wrong, $headers));
var_dump(openssl_pkcs7_encrypt($infile, $outfile, $empty, $headers));
var_dump(openssl_pkcs7_encrypt($infile, $outfile, $single_cert, $empty));
var_dump(openssl_pkcs7_encrypt($infile, $outfile, $multi_certs, $headers));

if (file_exists($outfile)) {
	echo "true\n";
	unlink($outfile);
}
if (file_exists($outfile2)) {
	echo "true\n";
	unlink($outfile2);
}
?>
