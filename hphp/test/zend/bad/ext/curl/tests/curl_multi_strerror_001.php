<?php

var_dump(strtolower(curl_multi_strerror(CURLM_OK)));
var_dump(strtolower(curl_multi_strerror(CURLM_BAD_HANDLE)));

?>