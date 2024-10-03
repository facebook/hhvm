<?hh <<__EntryPoint>> function main(): void {
$data = "openssl_encrypt() tests";
$method = "AES-128-CBC";
$password = "openssl";
$wrong = "wrong";
$object = new stdClass;
$arr = vec[1];

var_dump(openssl_encrypt($data, $wrong, $password));
}
