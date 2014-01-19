<?php

var_dump(strtolower(curl_strerror(CURLE_OK)));
var_dump(strtolower(curl_strerror(CURLE_UNSUPPORTED_PROTOCOL)));
var_dump(strtolower(curl_strerror(-1)));

?>