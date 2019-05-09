<?php
<<__EntryPoint>> function main() {
$a = 2;
openssl_pkcs12_read('1', &$a, '1');

echo "Done\n";
}
