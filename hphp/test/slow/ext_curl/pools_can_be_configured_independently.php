<?php
$ch1 = curl_init_pooled('pool1', "foo.bar");
$ch2 = curl_init_pooled('pool1', "foo.bar");
$ch3 = curl_init_pooled('pool2', "foo.bar");
echo "here\n";
$ch4 = curl_init_pooled('pool2', "foo.bar");
