<?hh <<__EntryPoint>> function main(): void {
for ($i = 0; $i < 10; $i++) {
    $strong = false;
    var_dump(bin2hex((string)openssl_random_pseudo_bytes($i, inout $strong)));
}
}
