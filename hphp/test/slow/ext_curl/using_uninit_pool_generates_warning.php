<?php
$ch = curl_init_pooled('nonextantpool', 'foo.com');
var_dump(curl_getinfo($ch)['url']);
curl_close($ch);
