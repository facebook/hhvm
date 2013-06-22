<?php

$sh = curl_share_init();
var_dump(curl_share_setopt($sh, CURLSHOPT_SHARE, CURL_LOCK_DATA_COOKIE));
var_dump(curl_share_setopt($sh, CURLSHOPT_UNSHARE, CURL_LOCK_DATA_DNS));
var_dump(curl_share_setopt($sh, -1, 0));

?>