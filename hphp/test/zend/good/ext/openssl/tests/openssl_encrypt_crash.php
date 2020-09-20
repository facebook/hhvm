<?hh <<__EntryPoint>> function main(): void {
openssl_encrypt('', 'AES-128-CBC', 'foo');
var_dump("done");
}
