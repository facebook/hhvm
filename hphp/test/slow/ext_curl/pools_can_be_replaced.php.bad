<?php

HH\curl_create_pool('unittest', 1);
$ch1 = HH\curl_init_pooled('unittest');
HH\curl_create_pool('unittest', 1, 2, 3);
// this is a new pool, so should succeed
$ch2 = HH\curl_init_pooled('unittest');
echo "here\n";
// this one should fail
$ch3 = HH\curl_init_pooled('unittest');
