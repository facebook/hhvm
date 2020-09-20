<?hh
<<__EntryPoint>> function main(): void {
$key = "012345678901234567890123";
var_dump(bin2hex(mcrypt_encrypt(MCRYPT_TRIPLEDES, $key, "data", MCRYPT_MODE_ECB)));
var_dump(bin2hex(mcrypt_encrypt(MCRYPT_TRIPLEDES, $key, "data", MCRYPT_MODE_ECB, "a")));
var_dump(bin2hex(mcrypt_encrypt(MCRYPT_TRIPLEDES, $key, "data", MCRYPT_MODE_ECB, "12345678")));
}
