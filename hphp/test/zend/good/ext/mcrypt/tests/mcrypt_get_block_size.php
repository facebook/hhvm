<?hh <<__EntryPoint>> function main(): void {
var_dump(mcrypt_get_block_size(MCRYPT_RIJNDAEL_256, MCRYPT_MODE_CBC));
var_dump(mcrypt_get_block_size(MCRYPT_3DES, MCRYPT_MODE_CBC));
var_dump(mcrypt_get_block_size(MCRYPT_WAKE, MCRYPT_MODE_STREAM));
}
