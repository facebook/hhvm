<?php
$curl = curl_init("http://www.google.com");
curl_setopt($curl, CURLOPT_RETURNTRANSFER, true);
curl_setopt($curl, CURLOPT_PRIVATE, "123");
curl_exec($curl);

var_dump(curl_getinfo($curl, CURLINFO_PRIVATE));