<?hh
<<__EntryPoint>> function main(): void {
$inputstr = file_get_contents(__DIR__ . "/bug74651.pem");
$pub_key_id = openssl_get_publickey($inputstr);
$sealed = null;
$ekeys = null;
$iv = null;
var_dump($pub_key_id);
var_dump(openssl_seal($inputstr, inout $sealed, inout $ekeys, vec[$pub_key_id, $pub_key_id], 'AES-128-ECB', inout $iv));
}
