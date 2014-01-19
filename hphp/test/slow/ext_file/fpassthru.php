<?php

$f = fopen(__DIR__.'/test_ext_file.txt', 'r');
fpassthru($f);
