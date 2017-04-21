<?php

$file = '/etc/passwd'.chr(0).'asdf';

var_dump(stream_resolve_include_path($file));
