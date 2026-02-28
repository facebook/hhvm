<?hh <<__EntryPoint>> function main(): void {
$data = "openssl_seal() test";
$cipher = 'AES-128-CBC';
$pub_key = "file://" . dirname(__FILE__) . "/public.key";
$priv_key = "file://" . dirname(__FILE__) . "/private.key";
$sealed = null;
$ekeys = null;
$iv = null;
$decrypted = null;

openssl_seal($data, inout $sealed, inout $ekeys, vec[$pub_key, $pub_key], $cipher, inout $iv);
openssl_seal($data, inout $sealed, inout $ekeys, vec[$pub_key, $pub_key],
             'sparkles', inout $iv);
openssl_seal($data, inout $sealed, inout $ekeys, vec[$pub_key, $pub_key],
             $cipher, inout $iv);
openssl_open($sealed, inout $decrypted, $ekeys[0], $priv_key, $cipher, $iv);
echo $decrypted;
}
