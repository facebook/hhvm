<?php

$fileResourceTemp = fopen('php://temp', 'wr');
stream_context_get_options($fileResourceTemp);
ftruncate($fileResourceTemp, PHP_INT_MAX);
?>