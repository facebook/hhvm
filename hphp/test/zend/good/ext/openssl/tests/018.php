<?hh <<__EntryPoint>> function main(): void {
$data = "Testing openssl_sign()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$wrong = "wrong";

$sign = null;
var_dump(openssl_sign($data, inout $sign, $privkey));                 // no output
var_dump(openssl_sign($data, inout $sign, $wrong));
}
