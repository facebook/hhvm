<?php

$tmp = tempnam(sys_get_temp_dir(), 'bad');
unlink($tmp);
var_dump(stream_resolve_include_path($tmp));
