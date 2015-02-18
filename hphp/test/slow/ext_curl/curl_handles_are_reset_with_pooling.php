<?php
$ch = curl_init_pooled('foo.bar.com');
curl_close($ch);
$ch = curl_init_pooled('therighturl.com');
var_dump(curl_getinfo($ch)['url']);
