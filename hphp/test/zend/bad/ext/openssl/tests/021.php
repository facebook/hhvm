<?php
$cert = "file://" . dirname(__FILE__) . "/cert.crt";
$priv = "file://" . dirname(__FILE__) . "/private.key";
$wrong = "wrong";
$pub = "file://" . dirname(__FILE__) . "/public.key";
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
	"config" => $config
	);

$privkey = openssl_pkey_new($config_arg);
$csr = openssl_csr_new($dn, $privkey, $args);
var_dump(openssl_csr_sign($csr, null, $privkey, 365, $args));
var_dump(openssl_csr_sign($csr, null, $privkey, 365, $config_arg));
var_dump(openssl_csr_sign($csr, $cert, $priv, 365, $config_arg));
var_dump(openssl_csr_sign($csr, $wrong, $privkey, 365));
var_dump(openssl_csr_sign($csr, null, $wrong, 365));
var_dump(openssl_csr_sign($csr, null, $privkey, $wrong));
var_dump(openssl_csr_sign($csr, null, $privkey, 365, $wrong));
var_dump(openssl_csr_sign($wrong, null, $privkey, 365));
var_dump(openssl_csr_sign(array(), null, $privkey, 365));
var_dump(openssl_csr_sign($csr, array(), $privkey, 365));
var_dump(openssl_csr_sign($csr, null, array(), 365));
var_dump(openssl_csr_sign($csr, null, $privkey, array()));
var_dump(openssl_csr_sign($csr, null, $privkey, 365, $config_arg));
?>