<?hh

<<__EntryPoint>>
function test_get_headers_secure_entrypoint() :mixed{
$headers = \HH\get_headers_secure();
ksort(inout $headers);
var_dump($headers);
}
