<?hh <<__EntryPoint>> function main(): void {
var_dump(mcrypt_module_get_algo_key_size(MCRYPT_RIJNDAEL_256));
var_dump(mcrypt_module_get_algo_key_size(MCRYPT_RIJNDAEL_192));
var_dump(mcrypt_module_get_algo_key_size(MCRYPT_RC2));
var_dump(mcrypt_module_get_algo_key_size(MCRYPT_XTEA));
var_dump(mcrypt_module_get_algo_key_size(MCRYPT_CAST_128));
var_dump(mcrypt_module_get_algo_key_size(MCRYPT_BLOWFISH));
}
