<?hh
<<__EntryPoint>> function main(): void {
$a = 2;
openssl_pkcs12_read('1', inout $a, '1');

echo "Done\n";
}
