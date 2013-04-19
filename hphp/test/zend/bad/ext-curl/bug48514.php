<?php

$ch1 = curl_init();
var_dump($ch1);
var_dump(get_resource_type($ch1));

$ch2 = curl_multi_init();
var_dump($ch2);
var_dump(get_resource_type($ch2));

?>