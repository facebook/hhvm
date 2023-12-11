<?hh <<__EntryPoint>> function main(): void {
$dir = dirname(__FILE__);
$certs = vec['bug39217cert2.txt', 'bug39217cert1.txt'];
foreach($certs as $cert) {
    $res = openssl_x509_parse(file_get_contents($dir . '/' . $cert));
    print_r($res['serialNumber']);
    echo "\n";
}
}
