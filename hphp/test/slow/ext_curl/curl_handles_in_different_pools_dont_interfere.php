<?php
$ch1 = curl_init_pooled('test', 'foo.com');
$ch2 = curl_init_pooled('test2', 'bar.com');
var_dump(curl_getinfo($ch1)['url']);
var_dump(curl_getinfo($ch2)['url']);
curl_close($ch1);
curl_close($ch2);
$ch3 = curl_init_pooled('test', 'foo.com');
$ch4 = curl_init_pooled('test', 'foo.com');
