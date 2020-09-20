<?hh <<__EntryPoint>> function main(): void {
$pfx = dirname(__FILE__) . DIRECTORY_SEPARATOR . "bug74022.pfx";
$cert_store = file_get_contents($pfx);

$cert_info = null;
var_dump(openssl_pkcs12_read($cert_store, inout $cert_info, "csos"));
var_dump(openssl_error_string());
}
