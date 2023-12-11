<?hh <<__EntryPoint>> function main(): void {
$data = "openssl_open() test";
$pub_key = "file://" . dirname(__FILE__) . "/public.key";
$priv_key = "file://" . dirname(__FILE__) . "/private.key";
$wrong = "wrong";
$sealed = null;
$ekeys = null;
$iv = null;
$output = null;
$output2 = null;
$output3 = null;
$output4 = null;

openssl_seal($data, inout $sealed, inout $ekeys, vec[$pub_key, $pub_key, $pub_key], '', inout $iv);
openssl_open($sealed, inout $output, $ekeys[0], $priv_key);
var_dump($output);
openssl_open($sealed, inout $output2, $ekeys[1], $wrong);
var_dump($output2);
openssl_open($sealed, inout $output3, $ekeys[2], $priv_key);
var_dump($output3);
openssl_open($sealed, inout $output4, $wrong, $priv_key);
var_dump($output4);
}
