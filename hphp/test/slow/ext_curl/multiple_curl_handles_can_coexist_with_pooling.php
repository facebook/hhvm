<?php
$ch = curl_init_pooled('test', 'foo.bar.com');
$ch2 = curl_init_pooled('test', 'www.baz.com');
var_dump(curl_getinfo($ch)['url']);
var_dump(curl_getinfo($ch2)['url']);
curl_close($ch);
curl_close($ch2);
