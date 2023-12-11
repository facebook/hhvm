<?hh <<__EntryPoint>> function main(): void {
$data = "openssl_open() test";
$pub_key = "file://" . dirname(__FILE__) . "/public.key";
$wrong = "wrong";
$sealed = null;
$ekeys = null;
$iv = null;

openssl_seal($data, inout $sealed, inout $ekeys, vec[$pub_key], '', inout $iv);                  // no output
openssl_seal($data, inout $sealed, inout $ekeys, vec[$pub_key, $pub_key], '', inout $iv);        // no output
openssl_seal($data, inout $sealed, inout $ekeys, vec[$pub_key, $wrong], '', inout $iv);
try { openssl_seal($data, inout $sealed, inout $ekeys, $pub_key, '', inout $iv); } catch (Exception $e) { echo "\n".'Warning: '.$e->getMessage().' in '.__FILE__.' on line '.__LINE__."\n"; }
openssl_seal($data, inout $sealed, inout $ekeys, vec[], '', inout $iv);
openssl_seal($data, inout $sealed, inout $ekeys, vec[$wrong], '', inout $iv);
}
