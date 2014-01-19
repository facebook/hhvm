<?php
$wrong = "wrong";
$config = __DIR__ . DIRECTORY_SEPARATOR . 'openssl.cnf';
$config_arg = array('config' => $config);

$dn = array(
	"countryName" => "BR",
	"stateOrProvinceName" => "Rio Grande do Sul",
	"localityName" => "Porto Alegre",
	"commonName" => "Henrique do N. Angelo",
	"emailAddress" => "hnangelo@php.net"
	);

$args = array(
	"digest_alg" => "sha1",
	"private_key_bits" => 2048,
	"private_key_type" => OPENSSL_KEYTYPE_DSA,
	"encrypt_key" => true,
	"config" => $config,
	);

$privkey = openssl_pkey_new($config_arg);
$csr = openssl_csr_new($dn, $privkey, $args);
var_dump(openssl_csr_export($csr, $output));
var_dump(openssl_csr_export($wrong, $output));
var_dump(openssl_csr_export($privkey, $output));
var_dump(openssl_csr_export(array(), $output));
var_dump(openssl_csr_export($csr, $output, false));
?>