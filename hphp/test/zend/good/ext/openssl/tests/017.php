<?hh <<__EntryPoint>> function main(): void {
$data = "Testing openssl_public_decrypt()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$pubkey = "file://" . dirname(__FILE__) . "/public.key";
$wrong = "wrong";

$encrypted = null;
$output = null;
$output2 = null;
$output3 = null;
$output4 = null;
$output5 = null;
openssl_public_encrypt($data, inout $encrypted, $pubkey);
var_dump(openssl_private_decrypt($encrypted, inout $output, $privkey));
var_dump($output);
var_dump(openssl_private_decrypt($encrypted, inout $output2, $wrong));
var_dump($output2);
var_dump(openssl_private_decrypt($wrong, inout $output3, $privkey));
var_dump($output3);
var_dump(openssl_private_decrypt($encrypted, inout $output4, vec[$privkey]));
var_dump($output4);
var_dump(openssl_private_decrypt($encrypted, inout $output5, vec[$privkey, ""]));
var_dump($output5);
}
