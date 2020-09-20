<?hh

<<__EntryPoint>>
function main_multiple_curl_handles_can_coexist_with_pooling() {
$ch = HH\curl_init_pooled('test', 'foo.bar.com');
$ch2 = HH\curl_init_pooled('test', 'www.baz.com');
var_dump(curl_getinfo($ch)['url']);
var_dump(curl_getinfo($ch2)['url']);
curl_close($ch);
curl_close($ch2);
}
