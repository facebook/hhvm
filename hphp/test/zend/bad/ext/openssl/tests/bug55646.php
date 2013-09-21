<?php
function stringAsHex($string){$unpacked = unpack("H*", $string);return implode(" ", str_split($unpacked[1],2));}

$config = array("digest_alg" => "sha1","x509_extensions" => "v3_ca","req_extensions" => "v3_req","private_key_bits" => 2048,"private_key_type" => OPENSSL_KEYTYPE_RSA,"encrypt_key" => false,);
$csr_info = array(
  "countryName" => "US",
  "stateOrProvinceName" => "Utah",
  "localityName" => "Lindon",
  "organizationName" => "Chinese",
  "organizationalUnitName" => "IT \xe4\xba\x92",
  "commonName" => "www.example.com",);
$private = openssl_pkey_new($config);
while (openssl_error_string()) {}
$csr_res = openssl_csr_new($csr_info, $private,
	['config' => __DIR__."/openssl.cnf"]);
if (!$csr_res) {
	while ($e = openssl_error_string()) { $err = $e; }
	die("Failed; last error: $err");
}
openssl_csr_export($csr_res, $csr);
$output = openssl_csr_get_subject($csr);

echo "A: ".$csr_info["organizationalUnitName"]."\n";
echo "B: ".stringAsHex($csr_info["organizationalUnitName"])."\n";
echo "C: ".$output['OU']."\n";
echo "D: ".stringAsHex($output['OU'])."\n";