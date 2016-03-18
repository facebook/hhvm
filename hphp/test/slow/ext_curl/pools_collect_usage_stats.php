<?php

HH\curl_create_pool('unittest', 1, 10);
$ch1 = HH\curl_init_pooled('unittest');
var_dump(HH\curl_list_pools()['unittest']['stats']);
try {
    $ch2 = HH\curl_init_pooled('unittest');
} catch(RuntimeException $e) {
    echo $e->getMessage(), "\n";
}
var_dump(HH\curl_list_pools()['unittest']['stats']);
