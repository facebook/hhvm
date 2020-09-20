<?hh


<<__EntryPoint>>
function main_pools_can_be_initialized_at_runtime() {
$pools = HH\curl_list_pools();
var_dump($pools);
var_dump(is_darray($pools));

HH\curl_create_pool('unittest', 10, 20, 30);
$pools = HH\curl_list_pools();
var_dump($pools);
var_dump(is_darray($pools));
var_dump(is_darray($pools['unittest']));

$ch1 = HH\curl_init_pooled('unittest');
HH\curl_destroy_pool('unittest');
$pools = HH\curl_list_pools();
var_dump($pools);
var_dump(is_darray($pools));
}
