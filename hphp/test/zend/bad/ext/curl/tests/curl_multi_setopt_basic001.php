<?php

$mh = curl_multi_init();
var_dump(curl_multi_setopt($mh, CURLMOPT_PIPELINING, 0));
var_dump(curl_multi_setopt($mh, -1, 0));

?>