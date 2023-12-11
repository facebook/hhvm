<?hh <<__EntryPoint>> function main(): void {
$data = "Testing openssl_sign()";
$privkey = "file://" . dirname(__FILE__) . "/private.key";
$wrong = "wrong";

$sign = null;
var_dump(openssl_sign($data, inout $sign, $privkey));                 // no output
var_dump(openssl_sign($data, inout $sign, $wrong));
try { var_dump(openssl_sign(vec[], inout $sign, $privkey)); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
}
