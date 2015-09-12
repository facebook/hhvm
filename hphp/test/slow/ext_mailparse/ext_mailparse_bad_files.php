<?php

$f = fopen('/etc/passwd', 'r');
fclose($f);
var_dump(mailparse_uudecode_all($f));
var_dump(mailparse_determine_best_xfer_encoding($f));
var_dump(mailparse_stream_encode($f, $f, 'utf8'));
