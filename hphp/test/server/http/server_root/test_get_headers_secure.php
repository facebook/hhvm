<?hh

$headers = \HH\get_headers_secure();
ksort(inout $headers);
var_dump($headers);
