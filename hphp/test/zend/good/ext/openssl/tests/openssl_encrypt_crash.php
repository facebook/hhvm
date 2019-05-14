<?php <<__EntryPoint>> function main() {
openssl_encrypt('', 'AES-128-CBC', 'foo');
var_dump("done");
}
