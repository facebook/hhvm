<?hh

<<__EntryPoint>>
function main_using_uninit_pool_generates_warning_and_returns_false() {
$ch = HH\curl_init_pooled('nonextantpool', 'foo.com');
var_dump($ch);
}
