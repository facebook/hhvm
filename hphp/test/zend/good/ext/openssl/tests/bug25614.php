<?hh <<__EntryPoint>> function main(): void {
$priv = openssl_pkey_new();
$pub = openssl_pkey_get_public($priv);
}
