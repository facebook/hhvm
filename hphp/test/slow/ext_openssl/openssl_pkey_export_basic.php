<?hh <<__EntryPoint>> function main(): void {
$key = openssl_pkey_get_private('file://' . dirname(__FILE__) . '/private_ec.key');
var_dump($key);

$config_arg = darray["config" => __DIR__ . DIRECTORY_SEPARATOR . "openssl.cnf"];

$output = null;
var_dump(openssl_pkey_export($key, inout $output, '', $config_arg));
// lintception
echo "// @lint-ignore-every PRIVATEKEY unit test\n";
echo $output;

// Load the private key from the exported pem string
$details = openssl_pkey_get_details(openssl_pkey_get_private($output));
var_dump(OPENSSL_KEYTYPE_EC === $details['type']);

// Export key with passphrase
openssl_pkey_export($key, inout $output, 'passphrase', $config_arg);

$details = openssl_pkey_get_details(openssl_pkey_get_private($output, 'passphrase'));
var_dump(OPENSSL_KEYTYPE_EC === $details['type']);

// Read public key
$pKey = openssl_pkey_get_public('file://' . dirname(__FILE__) . '/public_ec.key');
var_dump($pKey);
// The details are the same for a public or private key, expect the private key parameter 'd
$detailsPKey = openssl_pkey_get_details($pKey);
var_dump(array_diff_assoc($details['ec'], $detailsPKey['ec']));

// Export to file
$tempname = tempnam(sys_get_temp_dir(), 'openssl_ec');
var_dump(openssl_pkey_export_to_file($key, $tempname, '', $config_arg));
$details = openssl_pkey_get_details(openssl_pkey_get_private('file://' . $tempname));
var_dump(OPENSSL_KEYTYPE_EC === $details['type']);

// Clean the temporary file
unlink($tempname);
}
