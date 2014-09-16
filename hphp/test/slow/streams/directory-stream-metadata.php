<?php

$stream = opendir('.');
var_dump(stream_get_meta_data($stream));
