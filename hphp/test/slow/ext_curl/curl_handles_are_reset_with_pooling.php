<?hh

<<__EntryPoint>>
function main_curl_handles_are_reset_with_pooling() {
$ch = HH\curl_init_pooled('pool', 'foo.bar.com');
curl_close($ch);
$ch = HH\curl_init_pooled('pool', 'therighturl.com');
var_dump(curl_getinfo($ch)['url']);
}
