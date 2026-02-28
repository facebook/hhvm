<?hh

<<__EntryPoint>>
function main_http_build_query_enc_type() :mixed{
var_dump(PHP_QUERY_RFC1738);
var_dump(PHP_QUERY_RFC3986);

$data = dict['foo bar' => 'herp derp'];
var_dump(http_build_query($data, null, '', PHP_QUERY_RFC1738));
var_dump(http_build_query($data, null, '', PHP_QUERY_RFC3986));
var_dump(http_build_query($data, null, '', 42 /* invalid */));

$data = dict['model' => dict['foo bar' => 'herp derp']];
var_dump(http_build_query($data, null, '', PHP_QUERY_RFC1738));
var_dump(http_build_query($data, null, '', PHP_QUERY_RFC3986));
}
