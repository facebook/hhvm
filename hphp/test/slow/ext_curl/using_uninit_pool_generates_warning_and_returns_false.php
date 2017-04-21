<?php
$ch = HH\curl_init_pooled('nonextantpool', 'foo.com');
var_dump($ch);
