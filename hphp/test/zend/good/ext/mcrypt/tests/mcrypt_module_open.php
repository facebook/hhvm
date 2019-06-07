<?hh <<__EntryPoint>> function main() {
var_dump(mcrypt_module_open(MCRYPT_RIJNDAEL_256, '', MCRYPT_MODE_CBC, ''));
mcrypt_module_open('', '', '', '');
}
