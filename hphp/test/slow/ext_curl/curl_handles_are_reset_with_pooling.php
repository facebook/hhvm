<?php
$ch = HH\curl_init_pooled('pool', 'foo.bar.com');
curl_close($ch);
$ch = HH\curl_init_pooled('pool', 'therighturl.com');
var_dump(curl_getinfo($ch)['url']);
