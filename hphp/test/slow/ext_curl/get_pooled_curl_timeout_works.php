<?php
// the curl pool is 1, so the second request will fatal
$ch = curl_init_pooled('foo.bar.com');
$ch2 = curl_init_pooled('www.baz.com');
