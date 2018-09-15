<?php


<<__EntryPoint>>
function main_pools_can_be_initialized_at_runtime() {
var_dump(HH\curl_list_pools());
HH\curl_create_pool('unittest', 10, 20, 30);
var_dump(HH\curl_list_pools());
$ch1 = HH\curl_init_pooled('unittest');
HH\curl_destroy_pool('unittest');
var_dump(HH\curl_list_pools());
}
