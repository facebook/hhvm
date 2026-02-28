<?hh <<__EntryPoint>> function main(): void {
$configargs = dict[
        "req_extensions" => "v3_req",
        "x509_extensions" => "usr_cert",
        "config" => __DIR__."/openssl.cnf",
];

$dn = dict[
        "countryName" => "GB",
        "stateOrProvinceName" => "Berkshire",
        "localityName" => "Newbury",
        "organizationName" => "My Company Ltd",
        "commonName" => "Demo Cert"
];

$key = openssl_pkey_new();
$csr = openssl_csr_new($dn, inout $key, $configargs);
$crt = openssl_csr_sign($csr, NULL, $key, 365, $configargs);

$str = '';
openssl_csr_export($csr, inout $str, false);

if (strpos($str, 'Requested Extensions:')) {
    echo "Ok\n";
}
openssl_x509_export($crt, inout $str, false);
if (strpos($str, 'X509v3 extensions:')) {
    echo "Ok\n";
}
}
