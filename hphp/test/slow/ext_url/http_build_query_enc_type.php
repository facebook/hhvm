<?hh

<<__EntryPoint>>
function main_http_build_query_enc_type() {
var_dump(PHP_QUERY_RFC1738);
var_dump(PHP_QUERY_RFC3986);

$data = darray['foo bar' => 'herp derp'];
var_dump(http_build_query($data, null, '', PHP_QUERY_RFC1738));
var_dump(http_build_query($data, null, '', PHP_QUERY_RFC3986));
var_dump(http_build_query($data, null, '', 42 /* invalid */));

$data = darray['model' => darray['foo bar' => 'herp derp']];
var_dump(http_build_query($data, null, '', PHP_QUERY_RFC1738));
var_dump(http_build_query($data, null, '', PHP_QUERY_RFC3986));
}
