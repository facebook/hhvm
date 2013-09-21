<?php

//In January 2008 , level 7.18.0 of the curl lib, many of the messages changed.
//The final crlf was removed. This test is coded to work with or without the crlf.

$ch = curl_init();

curl_exec($ch);
var_dump(curl_error($ch));
var_dump(curl_errno($ch));
curl_close($ch);


?>