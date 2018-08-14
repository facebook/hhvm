<?hh

$headers = \HH\get_headers_secure();
ksort(&$headers);
var_dump($headers);
