<?hh

<<__EntryPoint>>
function test_proxygen_headers_entrypoint() :mixed{
$headers = \HH\get_proxygen_headers();
var_dump($headers);
}
